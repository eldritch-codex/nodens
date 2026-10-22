/// @file VulkanContext.cpp
/// @brief Implementation of Nodens' Vulkan context.
/// @ingroup Platform

module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <tracy/TracyVulkan.hpp>

module Nodens.VulkanContext;

import Nodens.Log;
import std;

namespace Nodens
{

/// @brief Per-frame Tracy Vulkan contexts. One per frame-in-flight command buffer.
struct GpuProfilerState
{
    std::array<TracyVkCtx, VulkanContext::FramesInFlight> Contexts{};
};

VulkanContext::VulkanContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
{
    if (!m_WindowHandle)
        throw std::invalid_argument{"VulkanContext requires a valid GLFW window"};
}

VulkanContext::~VulkanContext()
{
    if (!m_GpuProfiler)
        return;
    m_Device.waitIdle();
    for (TracyVkCtx context : m_GpuProfiler->Contexts)
        TracyVkDestroy(context);
}

void VulkanContext::Init()
{
    CreateInstance();
#if defined(ND_DEBUG)
    SetupDebugMessenger();
#endif
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateCommandResources();
    CreateFrameSynchronization();
#if defined(TRACY_ENABLE)
    InitializeGpuProfilerState();
#endif
}

void VulkanContext::CreateInstance()
{
    if (*m_Instance)
        return;

    constexpr vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Nodens",
        .applicationVersion = vk::makeApiVersion(0, 0, 0, 0),
        .pEngineName = "Nodens",
        .engineVersion = vk::makeApiVersion(0, 0, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    const auto layers = GetRequiredInstanceLayers();
    const auto extensions = GetRequiredInstanceExtensions();

    const vk::InstanceCreateInfo instanceCreateInfo{
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    m_Instance = vk::raii::Instance{m_Context, instanceCreateInfo};
}

std::vector<const char*> VulkanContext::GetRequiredInstanceLayers() const
{
    std::vector<const char*> layers{};
#if defined(ND_DEBUG)
    char const* validationLayer{"VK_LAYER_KHRONOS_validation"};

    // List available layers, then check the required one is present.
    const auto availableLayers =
        m_Context.enumerateInstanceLayerProperties() |
        std::views::transform([](const auto& prop) { return std::string_view{prop.layerName}; });
    if (!std::ranges::contains(availableLayers, std::string_view{validationLayer}))
        throw std::runtime_error{"VK_LAYER_KHRONOS_validation layer is not available. Required for "
                                 "Nodens Debug config."};

    layers.push_back(validationLayer);
#endif
    return layers;
}

std::vector<const char*> VulkanContext::GetRequiredInstanceExtensions() const
{
    uint32_t glfwExtensionsCount{0};
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    if (!glfwExtensions)
        throw std::runtime_error("GLFW Vulkan instance extensions are unavailable");

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
#if defined(ND_DEBUG)
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    // List available extensions, then check every required one is present.
    const auto availableExtensions =
        m_Context.enumerateInstanceExtensionProperties() |
        std::views::transform([](const auto& prop)
                              { return std::string_view{prop.extensionName}; });
    const auto unsupported = std::ranges::find_if(
        extensions,
        [&](const char* required)
        { return !std::ranges::contains(availableExtensions, std::string_view{required}); });
    if (unsupported != extensions.end())
        throw std::runtime_error{"Required instance extension not supported: " +
                                 std::string{*unsupported}};

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                             vk::DebugUtilsMessageTypeFlagsEXT type,
                                             const vk::DebugUtilsMessengerCallbackDataEXT* data,
                                             void* userData)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        Nodens::CoreLogger().error("Vulkan validation: {}", data->pMessage);
    else
        Nodens::CoreLogger().warn("Vulkan validation: {}", data->pMessage);
    return VK_FALSE;
}

void VulkanContext::SetupDebugMessenger()
{
    m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT({
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = DebugCallback,
    });
}

void VulkanContext::CreateSurface()
{
    VkSurfaceKHR rawSurface{VK_NULL_HANDLE};
    if (glfwCreateWindowSurface(*m_Instance, m_WindowHandle, nullptr, &rawSurface) != VK_SUCCESS)
        throw std::runtime_error{"Failed to create Vulkan window surface"};
    m_Surface = vk::raii::SurfaceKHR{m_Instance, rawSurface};
}

void VulkanContext::PickPhysicalDevice()
{
    const auto availablePhysicalDevices = m_Instance.enumeratePhysicalDevices();
    const auto deviceIterator = std::ranges::find_if(
        availablePhysicalDevices, [&](const auto& device) { return IsDeviceSuitable(device); });
    if (deviceIterator == availablePhysicalDevices.end())
        throw std::runtime_error{"Nodens found no suitable Vulkan physical device"};

    m_PhysicalDevice = *deviceIterator;

    const vk::PhysicalDeviceProperties properties = m_PhysicalDevice.getProperties();
    CoreLogger().info("Vulkan Info:");
    CoreLogger().info("  GPU Used: {}", std::string_view{properties.deviceName});
    CoreLogger().info("  Vendor ID: 0x{:04x}", properties.vendorID);
    CoreLogger().info("  Device Type: {}", vk::to_string(properties.deviceType));
    CoreLogger().info("  API Version: {}.{}.{}",
                      vk::apiVersionMajor(properties.apiVersion),
                      vk::apiVersionMinor(properties.apiVersion),
                      vk::apiVersionPatch(properties.apiVersion));
    CoreLogger().info("  Driver Version: {}", properties.driverVersion);
}

bool VulkanContext::IsDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) const
{
    if (physicalDevice.getProperties().apiVersion < vk::ApiVersion13)
        return false;

    const auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    const bool supportsGraphicsAndPresentation = std::ranges::any_of(
        std::views::iota(size_t{0}, queueFamilies.size()),
        [&](size_t index)
        {
            const auto& family = queueFamilies[index];
            if (!(family.queueFlags & vk::QueueFlagBits::eGraphics))
                return false;
            if (!physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(index), *m_Surface))
                return false;
#if defined(TRACY_ENABLE)
            // Timestamp ticks cannot convert to elapsed time.
            if (physicalDevice.getProperties().limits.timestampPeriod <= 0.0f)
                return false;
            // Queue cannot write usable timestamp queries.
            if (family.timestampValidBits == 0)
                return false;
#endif
            return true;
        });
    if (!supportsGraphicsAndPresentation)
        return false;

    std::vector<const char*> requiredExtensions{vk::KHRSwapchainExtensionName};
#if defined(TRACY_ENABLE)
    requiredExtensions.push_back(vk::EXTCalibratedTimestampsExtensionName);
#endif
    const auto availableExtensions =
        physicalDevice.enumerateDeviceExtensionProperties() |
        std::views::transform([](const auto& property)
                              { return std::string_view{property.extensionName}; });
    bool supportsAllRequiredExtensions =
        std::ranges::all_of(requiredExtensions,
                            [&](std::string_view required)
                            { return std::ranges::contains(availableExtensions, required); });
    if (!supportsAllRequiredExtensions)
        return false;

    const auto features =
        physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                             vk::PhysicalDeviceVulkan11Features,
                                             vk::PhysicalDeviceVulkan13Features,
                                             vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    return features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
           features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
           features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
           features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
               .extendedDynamicState;
}

void VulkanContext::CreateLogicalDevice()
{
    const auto queueFamilies = m_PhysicalDevice.getQueueFamilyProperties();
    const auto queueIterator = std::ranges::find_if(
        std::views::iota(size_t{0}, queueFamilies.size()),
        [&](size_t index)
        {
            const auto& family = queueFamilies[index];

            if (!(family.queueFlags & vk::QueueFlagBits::eGraphics))
                return false;
            if (!m_PhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(index), *m_Surface))
                return false;

#if defined(TRACY_ENABLE)
            if (family.timestampValidBits == 0)
                return false;
            if (m_PhysicalDevice.getProperties().limits.timestampPeriod <= 0.0f)
                return false;
#endif

            return true;
        });

    if (queueIterator == std::views::iota(size_t{0}, queueFamilies.size()).end())
        throw std::runtime_error{"Nodens found no graphics and presentation queue"};

    m_GraphicsQueueFamilyIndex = static_cast<uint32_t>(*queueIterator);
    constexpr float queuePriority{1.0f};
    const vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = m_GraphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {{},
                        {.shaderDrawParameters = true},
                        {.synchronization2 = true, .dynamicRendering = true},
                        {.extendedDynamicState = true}};

    std::vector<const char*> requiredExtensions{vk::KHRSwapchainExtensionName};
#if defined(TRACY_ENABLE)
    requiredExtensions.push_back(vk::EXTCalibratedTimestampsExtensionName);
#endif
    const vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    m_Device = vk::raii::Device{m_PhysicalDevice, deviceCreateInfo};
    m_GraphicsQueue = vk::raii::Queue{m_Device, m_GraphicsQueueFamilyIndex, 0};
}

void VulkanContext::CreateSwapchain()
{
    const auto capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
    m_SwapchainExtent = ChooseSwapchainExtent(capabilities);
    const auto formats = m_PhysicalDevice.getSurfaceFormatsKHR(*m_Surface);
    m_SwapchainSurfaceFormat = ChooseSwapchainSurfaceFormat(formats);
    const auto presentModes = m_PhysicalDevice.getSurfacePresentModesKHR(*m_Surface);

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = *m_Surface,
        .minImageCount = ChooseSwapchainImageCount(capabilities),
        .imageFormat = m_SwapchainSurfaceFormat.format,
        .imageColorSpace = m_SwapchainSurfaceFormat.colorSpace,
        .imageExtent = m_SwapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = ChooseSwapchainPresentMode(presentModes),
        .clipped = true,
    };

    m_Swapchain = vk::raii::SwapchainKHR{m_Device, createInfo};
    m_SwapchainImages = m_Swapchain.getImages();
    // A recreated swapchain has new images, so reset layout tracking with each image set.
    m_SwapchainImageLayouts.assign(m_SwapchainImages.size(), vk::ImageLayout::eUndefined);

    const vk::ImageViewCreateInfo imageViewInfo{
        .viewType = vk::ImageViewType::e2D,
        .format = m_SwapchainSurfaceFormat.format,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
    };
    for (const auto image : m_SwapchainImages)
    {
        auto viewInfo = imageViewInfo;
        viewInfo.image = image;
        m_SwapchainImageViews.emplace_back(m_Device, viewInfo);
    }
}

vk::Extent2D
VulkanContext::ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width{0};
    int height{0};
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    return {.width = std::clamp<uint32_t>(
                width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp<uint32_t>(
                height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
}

vk::SurfaceFormatKHR VulkanContext::ChooseSwapchainSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
    assert(!availableFormats.empty());
    const auto formatIterator =
        std::ranges::find_if(availableFormats,
                             [](const vk::SurfaceFormatKHR& format)
                             {
                                 return format.format == vk::Format::eB8G8R8A8Unorm &&
                                        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                             });
    return formatIterator != availableFormats.end() ? *formatIterator : availableFormats.front();
}

uint32_t
VulkanContext::ChooseSwapchainImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    auto imageCount = std::max(3u, capabilities.minImageCount);
    if (capabilities.maxImageCount != 0 && capabilities.maxImageCount < imageCount)
        imageCount = capabilities.maxImageCount;
    return imageCount;
}

vk::PresentModeKHR VulkanContext::ChooseSwapchainPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) const
{
    assert(std::ranges::any_of(availablePresentModes,
                               [](vk::PresentModeKHR mode)
                               { return mode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(availablePresentModes,
                               [](vk::PresentModeKHR mode)
                               { return mode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

void VulkanContext::CreateCommandResources()
{
    const vk::CommandPoolCreateInfo poolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_GraphicsQueueFamilyIndex,
    };
    m_CommandPool = vk::raii::CommandPool{m_Device, poolCreateInfo};

    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = FramesInFlight,
    };
    m_CommandBuffers = vk::raii::CommandBuffers{m_Device, allocateInfo};
}

void VulkanContext::CreateFrameSynchronization()
{
    CreateRenderFinishedSemaphores();
    for (uint32_t index = 0; index < FramesInFlight; ++index)
    {
        m_PresentCompleteSemaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo{});
        m_InFlightFences.emplace_back(
            m_Device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void VulkanContext::CreateRenderFinishedSemaphores()
{
    m_RenderFinishedSemaphores.clear();
    for (size_t index = 0; index < m_SwapchainImages.size(); ++index)
        m_RenderFinishedSemaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo{});
}

void VulkanContext::InitializeGpuProfilerState()
{
    // Initialize GPU profiler state.
    m_GpuProfiler = std::make_unique<GpuProfilerState>();
    for (uint32_t index = 0; index < VulkanContext::FramesInFlight; ++index)
    {
        m_GpuProfiler->Contexts[index] = TracyVkContextCalibrated(
            *m_PhysicalDevice,
            *m_Device,
            *m_GraphicsQueue,
            *m_CommandBuffers[index],
            m_Instance.getDispatcher()->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
            m_Device.getDispatcher()->vkGetCalibratedTimestampsEXT);
    }
}

std::optional<uint32_t> VulkanContext::BeginFrame()
{
    int framebufferWidth{0};
    int framebufferHeight{0};
    glfwGetFramebufferSize(m_WindowHandle, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth == 0 || framebufferHeight == 0)
        return std::nullopt;

    // Recreate before recording when a resize leaves the swapchain extent stale.
    const auto surfaceCapabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
    const auto expectedExtent = ChooseSwapchainExtent(surfaceCapabilities);
    if (expectedExtent.width != m_SwapchainExtent.width ||
        expectedExtent.height != m_SwapchainExtent.height)
    {
        RecreateSwapchain();
        return std::nullopt;
    }

    const auto fenceResult = m_Device.waitForFences(
        *m_InFlightFences[m_CurrentFrameIndex], vk::True, std::numeric_limits<uint64_t>::max());
    if (fenceResult != vk::Result::eSuccess)
        throw std::runtime_error{"Nodens failed to wait for frame fence"};

    auto [result, imageIndex] =
        m_Swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(),
                                     *m_PresentCompleteSemaphores[m_CurrentFrameIndex],
                                     nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapchain();
        return std::nullopt;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error{"Nodens failed to acquire swapchain image"};

    m_Device.resetFences(*m_InFlightFences[m_CurrentFrameIndex]);
    m_CurrentImageIndex = imageIndex;
    m_CommandBuffers[m_CurrentFrameIndex].reset();
    m_CommandBuffers[m_CurrentFrameIndex].begin({});
    TransitionActiveImageToColorAttachment();
    m_FrameStarted = true;
    if (m_GpuProfiler)
    {
        TracyVkCollect(m_GpuProfiler->Contexts[m_CurrentFrameIndex],
                       *m_CommandBuffers[m_CurrentFrameIndex]);
    }
    return imageIndex;
}

void VulkanContext::TransitionActiveImageToColorAttachment()
{
    const vk::ImageMemoryBarrier2 barrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .srcAccessMask = {},
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask =
            vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = m_SwapchainImageLayouts[m_CurrentImageIndex],
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = m_SwapchainImages[m_CurrentImageIndex],
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
    };
    const vk::DependencyInfo dependencyInfo{.imageMemoryBarrierCount = 1,
                                            .pImageMemoryBarriers = &barrier};
    m_CommandBuffers[m_CurrentFrameIndex].pipelineBarrier2(dependencyInfo);
    m_SwapchainImageLayouts[m_CurrentImageIndex] = vk::ImageLayout::eColorAttachmentOptimal;
}

bool VulkanContext::IsFrameActive() const
{
    return m_FrameStarted;
}

const vk::raii::CommandBuffer& VulkanContext::GetActiveCommandBuffer() const
{
    return m_CommandBuffers[m_CurrentFrameIndex];
}

void VulkanContext::TransitionActiveImageToPresent()
{
    const vk::ImageMemoryBarrier2 barrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .dstAccessMask = {},
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = m_SwapchainImages[m_CurrentImageIndex],
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
    };
    const vk::DependencyInfo dependencyInfo{.imageMemoryBarrierCount = 1,
                                            .pImageMemoryBarriers = &barrier};
    m_CommandBuffers[m_CurrentFrameIndex].pipelineBarrier2(dependencyInfo);
    m_SwapchainImageLayouts[m_CurrentImageIndex] = vk::ImageLayout::ePresentSrcKHR;
}

void VulkanContext::Present()
{
    if (!m_FrameStarted)
        return;

    TransitionActiveImageToPresent();
    auto& commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];
    commandBuffer.end();
    const vk::CommandBuffer rawCommandBuffer = *commandBuffer;

    const vk::PipelineStageFlags waitStage{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_PresentCompleteSemaphores[m_CurrentFrameIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &rawCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_RenderFinishedSemaphores[m_CurrentImageIndex],
    };
    m_GraphicsQueue.submit(submitInfo, *m_InFlightFences[m_CurrentFrameIndex]);

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_RenderFinishedSemaphores[m_CurrentImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*m_Swapchain,
        .pImageIndices = &m_CurrentImageIndex,
    };
    const auto result = m_GraphicsQueue.presentKHR(presentInfo);
    m_FrameStarted = false;

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        RecreateSwapchain();
    else if (result != vk::Result::eSuccess)
        throw std::runtime_error{"Nodens failed to present swapchain image"};
    else
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FramesInFlight;
}

void VulkanContext::RecreateSwapchain()
{
    m_Device.waitIdle();
    m_SwapchainImageViews.clear();
    m_RenderFinishedSemaphores.clear();
    m_Swapchain = nullptr;
    CreateSwapchain();
    CreateRenderFinishedSemaphores();
}

void VulkanContext::WaitIdle() const
{
    m_Device.waitIdle();
}

void* VulkanContext::GetGpuProfilerContext(uint32_t frameIndex) const
{
    if (!m_GpuProfiler || frameIndex >= VulkanContext::FramesInFlight)
        return nullptr;
    return m_GpuProfiler->Contexts[frameIndex];
}

vk::Instance VulkanContext::GetInstance() const
{
    return *m_Instance;
}

vk::SurfaceKHR VulkanContext::GetSurface() const
{
    return *m_Surface;
}

const vk::raii::Instance& VulkanContext::GetInstanceRAII() const
{
    return m_Instance;
}

const vk::raii::SurfaceKHR& VulkanContext::GetSurfaceRAII() const
{
    return m_Surface;
}

const vk::raii::PhysicalDevice& VulkanContext::GetPhysicalDevice() const
{
    return m_PhysicalDevice;
}

const vk::raii::Device& VulkanContext::GetDeviceRAII() const
{
    return m_Device;
}

const vk::raii::Queue& VulkanContext::GetGraphicsQueueRAII() const
{
    return m_GraphicsQueue;
}

uint32_t VulkanContext::GetGraphicsQueueFamilyIndex() const
{
    return m_GraphicsQueueFamilyIndex;
}

const vk::raii::SwapchainKHR& VulkanContext::GetSwapchainRAII() const
{
    return m_Swapchain;
}

const std::vector<vk::Image>& VulkanContext::GetSwapchainImages() const
{
    return m_SwapchainImages;
}

const std::vector<vk::raii::ImageView>& VulkanContext::GetSwapchainImageViews() const
{
    return m_SwapchainImageViews;
}

vk::Extent2D VulkanContext::GetSwapchainExtent() const
{
    return m_SwapchainExtent;
}

vk::SurfaceFormatKHR VulkanContext::GetSwapchainSurfaceFormat() const
{
    return m_SwapchainSurfaceFormat;
}

uint32_t VulkanContext::GetFramesInFlight() const
{
    return FramesInFlight;
}

uint32_t VulkanContext::GetCurrentFrameIndex() const
{
    return m_CurrentFrameIndex;
}

uint32_t VulkanContext::GetCurrentImageIndex() const
{
    return m_CurrentImageIndex;
}

} // namespace Nodens
