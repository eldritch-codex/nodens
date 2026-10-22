/// @file VulkanImGuiRenderer.cpp
/// @brief Implementation of the Dear ImGui Vulkan renderer backend.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <tracy/TracyVulkan.hpp>

module Nodens.VulkanImGuiRenderer;

namespace Nodens
{

void VulkanImGuiRenderer::Init(GLFWwindow* window)
{
    ImGui_ImplGlfw_InitForVulkan(window, true);

    const VkFormat colorFormat =
        static_cast<VkFormat>(m_Context.GetSwapchainSurfaceFormat().format);
    // The backend creates its pipeline from this format, so it must match Nodens' swapchain image.
    const VkPipelineRenderingCreateInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
    };

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = static_cast<VkInstance>(m_Context.GetInstance());
    initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(*m_Context.GetPhysicalDevice());
    initInfo.Device = static_cast<VkDevice>(*m_Context.GetDeviceRAII());
    initInfo.QueueFamily = m_Context.GetGraphicsQueueFamilyIndex();
    initInfo.Queue = static_cast<VkQueue>(*m_Context.GetGraphicsQueueRAII());
    initInfo.DescriptorPoolSize = 1000;
    initInfo.MinImageCount = m_Context.GetFramesInFlight();
    initInfo.ImageCount = static_cast<uint32_t>(m_Context.GetSwapchainImages().size());
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = renderingInfo;

    if (!ImGui_ImplVulkan_Init(&initInfo))
        throw std::runtime_error{"Failed to initialize Dear ImGui Vulkan backend"};
}

void VulkanImGuiRenderer::Shutdown()
{
    m_Context.WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void VulkanImGuiRenderer::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // GLFW reports logical coordinates; keep DisplaySize for input and scale rasterization to
    // the physical framebuffer pixels used by the Vulkan swapchain.
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f)
    {
        const vk::Extent2D framebufferExtent = m_Context.GetSwapchainExtent();
        io.DisplayFramebufferScale = ImVec2{
            static_cast<float>(framebufferExtent.width) / io.DisplaySize.x,
            static_cast<float>(framebufferExtent.height) / io.DisplaySize.y,
        };
    }
}

void VulkanImGuiRenderer::RenderDrawData(ImDrawData* drawData)
{
    if (!drawData || drawData->TotalVtxCount == 0 || !m_Context.IsFrameActive())
        return;

    auto& commandBuffer = m_Context.GetActiveCommandBuffer();
    // Nyar rendered this attachment earlier in the same command buffer; make its writes visible
    // before ImGui loads the image and overlays its draw data.
    const vk::ImageMemoryBarrier2 barrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask =
            vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = m_Context.GetSwapchainImages()[m_Context.GetCurrentImageIndex()],
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
    };
    const vk::DependencyInfo dependencyInfo{.imageMemoryBarrierCount = 1,
                                            .pImageMemoryBarriers = &barrier};
    commandBuffer.pipelineBarrier2(dependencyInfo);

    const vk::RenderingAttachmentInfo attachment{
        .imageView = *m_Context.GetSwapchainImageViews()[m_Context.GetCurrentImageIndex()],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };
    const vk::RenderingInfo renderingInfo{
        .renderArea = {.offset = {0, 0}, .extent = m_Context.GetSwapchainExtent()},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment,
    };

    TracyVkZone(
        static_cast<TracyVkCtx>(m_Context.GetGpuProfilerContext(m_Context.GetCurrentFrameIndex())),
        *commandBuffer,
        "ImGui");

    commandBuffer.beginRendering(renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(drawData, *commandBuffer);
    commandBuffer.endRendering();
}

} // namespace Nodens
