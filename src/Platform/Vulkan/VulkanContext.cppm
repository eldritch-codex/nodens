/// @file VulkanContext.cppm
/// @brief Nodens-owned Vulkan instance, device, surface, and swapchain context.
/// @details Nodens owns loader, instance, surface, physical-device selection,
///          logical-device creation, and swapchain resources. Frame scheduling
///          follows later.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>

export module Nodens.VulkanContext;

import Nodens.GraphicsContext;
import std;
export import vulkan;

namespace Nodens
{
/// @brief Opaque owner of the per-frame Tracy Vulkan profiling contexts.
/// @details Defined in VulkanContext.cpp so Tracy headers stay out of this module.
struct GpuProfilerState;
} // namespace Nodens

export namespace Nodens
{
/// @brief Vulkan context shared by Nodens and an attached renderer.
/// @details Nodens owns Vulkan platform and swapchain resources. Consumers borrow
///          these handles and must not destroy them. The context requires a GLFW window
///          created with `GLFW_NO_API`.
/// @ingroup Platform
class VulkanContext : public IGraphicsContext
{
public:
    static constexpr uint32_t FramesInFlight{2};

    /// @brief Constructs a Vulkan context for an existing GLFW window.
    /// @param windowHandle A valid GLFW window created without a client API.
    explicit VulkanContext(GLFWwindow* windowHandle);

    ~VulkanContext() override;

    /// @brief Creates the Vulkan instance, surface, physical device, and logical device.
    void Init() override;

    /// @brief Acquires a swapchain image and starts one frame.
    /// @return Image index, or no value when swapchain recreation is required.
    std::optional<uint32_t> BeginFrame();

    /// @brief Reports whether BeginFrame() started a frame for recording.
    bool IsFrameActive() const;

    /// @brief Returns command buffer receiving commands for the active frame.
    /// @return Borrowed reference to the active RAII command buffer.
    const vk::raii::CommandBuffer& GetActiveCommandBuffer() const;

    /// @brief Transitions the active swapchain image to the presentation layout.
    /// @details Called by Present() after all renderers finish recording. Renderers can
    ///          therefore append work after Nyar's scene pass without ending the frame.
    void TransitionActiveImageToPresent();

    /// @brief Completes, submits, and presents the active frame.
    void Present() override;

    /// @brief Recreates swapchain resources after a surface change.
    void RecreateSwapchain();

    /// @brief Waits until all Nodens-owned Vulkan work completes.
    void WaitIdle() const;

    /// @brief Returns the Tracy Vulkan context for a frame slot.
    /// @return TracyVkCtx handle, or null when profiling is not compiled in.
    void* GetGpuProfilerContext(uint32_t frameIndex) const;

    /// @brief Returns the Vulkan instance owned by Nodens.
    /// @return Borrowed Vulkan instance handle.
    vk::Instance GetInstance() const;

    /// @brief Returns the Vulkan surface owned by Nodens.
    /// @return Borrowed Vulkan surface handle.
    vk::SurfaceKHR GetSurface() const;

    /// @brief Returns the Vulkan-Hpp RAII instance owned by Nodens.
    /// @return Borrowed reference to the RAII instance.
    const vk::raii::Instance& GetInstanceRAII() const;

    /// @brief Returns the Vulkan-Hpp RAII surface owned by Nodens.
    /// @return Borrowed reference to the RAII surface.
    const vk::raii::SurfaceKHR& GetSurfaceRAII() const;

    /// @brief Returns the physical device selected by Nodens.
    /// @return Borrowed reference to the selected physical device.
    const vk::raii::PhysicalDevice& GetPhysicalDevice() const;

    /// @brief Returns the logical device created by Nodens.
    /// @return Borrowed reference to the logical device.
    const vk::raii::Device& GetDeviceRAII() const;

    /// @brief Returns the graphics and presentation queue created by Nodens.
    /// @return Borrowed reference to the queue.
    const vk::raii::Queue& GetGraphicsQueueRAII() const;

    /// @brief Returns the graphics and presentation queue family index.
    uint32_t GetGraphicsQueueFamilyIndex() const;

    /// @brief Returns the Nodens-owned swapchain.
    /// @return Borrowed reference to the swapchain.
    const vk::raii::SwapchainKHR& GetSwapchainRAII() const;

    /// @brief Returns swapchain image handles owned by Nodens.
    /// @return Borrowed reference to swapchain images.
    const std::vector<vk::Image>& GetSwapchainImages() const;

    /// @brief Returns swapchain image views owned by Nodens.
    /// @return Borrowed reference to swapchain image views.
    const std::vector<vk::raii::ImageView>& GetSwapchainImageViews() const;

    /// @brief Returns current swapchain extent.
    vk::Extent2D GetSwapchainExtent() const;

    /// @brief Returns current swapchain surface format.
    vk::SurfaceFormatKHR GetSwapchainSurfaceFormat() const;

    /// @brief Returns number of frames Nodens schedules concurrently.
    uint32_t GetFramesInFlight() const;

    /// @brief Returns current frame slot used by Nodens.
    uint32_t GetCurrentFrameIndex() const;

    /// @brief Returns the swapchain image acquired for the active frame.
    uint32_t GetCurrentImageIndex() const;

private:
    /// @brief Creates the Vulkan instance for the context.
    void CreateInstance();

    /// @brief Returns the required instance layers for the Vulkan context.
    /// @details For now the only required layers are validation layers, enabled only in debug mode.
    std::vector<const char*> GetRequiredInstanceLayers() const;

    /// @brief Returns the required instance extensions for the Vulkan context.
    std::vector<const char*> GetRequiredInstanceExtensions() const;

    /// @brief Sets up the debug messenger for the Vulkan context.
    void SetupDebugMessenger();

    /// @brief Creates the Vulkan surface for the window.
    void CreateSurface();

    /// @brief Selects the first physical device meeting current requirements.
    void PickPhysicalDevice();

    /// @brief Checks whether a physical device meets current Nyar requirements.
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) const;

    /// @brief Creates logical device with one graphics-and-present queue.
    void CreateLogicalDevice();

    /// @brief Creates swapchain and image views for the current surface.
    void CreateSwapchain();

    /// @brief Clamps framebuffer size to surface limits when needed.
    vk::Extent2D ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

    /// @brief Prefers an UNORM color format for Dear ImGui's linear shader output.
    vk::SurfaceFormatKHR
    ChooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;

    /// @brief Chooses at least three swapchain images within surface limits.
    uint32_t ChooseSwapchainImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) const;

    /// @brief Prefers mailbox presentation and falls back to FIFO.
    vk::PresentModeKHR
    ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;

    /// @brief Creates command pool and one command buffer per frame in flight.
    void CreateCommandResources();

    /// @brief Creates synchronization objects for each frame in flight.
    void CreateFrameSynchronization();

    /// @brief Creates one render-finished semaphore per swapchain image.
    void CreateRenderFinishedSemaphores();

    /// @brief Initializes GPU profiler state.
    void InitializeGpuProfilerState();

    /// @brief Transitions the acquired swapchain image for color rendering.
    /// @details The acquired image can come from any swapchain slot, so its tracked previous
    ///          layout is used instead of assuming every acquisition starts undefined.
    void TransitionActiveImageToColorAttachment();

    GLFWwindow* m_WindowHandle{nullptr}; ///< Borrowed GLFW window owned by IWindow.
    vk::raii::Context m_Context{};       ///< Vulkan loader context.
    vk::raii::Instance m_Instance{nullptr};             ///< Nodens-owned Vulkan instance.
    vk::raii::SurfaceKHR m_Surface{nullptr};            ///< Nodens-owned Vulkan surface.
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger{
        nullptr}; ///< Debug messenger for Vulkan validation.
    vk::raii::PhysicalDevice m_PhysicalDevice{nullptr}; ///< Nodens-selected physical device.
    vk::raii::Device m_Device{nullptr};                 ///< Nodens-owned logical device.
    vk::raii::Queue m_GraphicsQueue{nullptr};           ///< Graphics and presentation queue.
    uint32_t m_GraphicsQueueFamilyIndex{};              ///< Graphics and presentation queue family.
    vk::raii::SwapchainKHR m_Swapchain{nullptr};        ///< Nodens-owned presentation swapchain.
    vk::Extent2D m_SwapchainExtent{};                   ///< Current swapchain dimensions.
    vk::SurfaceFormatKHR m_SwapchainSurfaceFormat{};    ///< Current swapchain format.
    std::vector<vk::Image> m_SwapchainImages{};         ///< Swapchain image handles.
    std::vector<vk::ImageLayout> m_SwapchainImageLayouts{}; ///< Current layout per swapchain image.
    std::vector<vk::raii::ImageView> m_SwapchainImageViews{}; ///< Nodens-owned image views.
    vk::raii::CommandPool m_CommandPool{nullptr};             ///< Nodens-owned command pool.
    std::vector<vk::raii::CommandBuffer>
        m_CommandBuffers{}; ///< Nodens-owned frame command buffers.
    std::vector<vk::raii::Semaphore> m_PresentCompleteSemaphores{}; ///< Image-acquire signals.
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores{};  ///< Render-complete signals.
    std::vector<vk::raii::Fence> m_InFlightFences{};                ///< CPU/GPU frame fences.
    uint32_t m_CurrentFrameIndex{0};                 ///< Current frame-in-flight slot.
    uint32_t m_CurrentImageIndex{0};                 ///< Current acquired image.
    bool m_FrameStarted{false};                      ///< BeginFrame has acquired an image.
    std::unique_ptr<GpuProfilerState> m_GpuProfiler; ///< Per-frame GPU profiling contexts.
};
} // namespace Nodens
