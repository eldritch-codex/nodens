/// @file VulkanImGuiRenderer.cppm
/// @brief Vulkan implementation of the ImGuiRenderer interface.
/// @details Uses Nodens-owned Vulkan objects while recording ImGui draw data into
///          the active Nodens command buffer.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>
#include <imgui.h>

export module Nodens.VulkanImGuiRenderer;

import Nodens.ImGuiRenderer;
import Nodens.VulkanContext;

export namespace Nodens
{
/// @brief Concrete ImGuiRenderer implementation using Dear ImGui's Vulkan backend.
/// @details Borrows all Vulkan objects from VulkanContext. The renderer does not own
///          the instance, device, queue, swapchain, or command buffers.
/// @see ImGuiRenderer, VulkanContext, ImGuiLayer
/// @ingroup Platform
class VulkanImGuiRenderer : public ImGuiRenderer
{
public:
    /// @brief Constructs a renderer that borrows a Nodens Vulkan context.
    /// @param context Nodens-owned Vulkan context with an initialized swapchain.
    explicit VulkanImGuiRenderer(VulkanContext& context) : m_Context(context)
    {
    }

    ~VulkanImGuiRenderer() override = default;

    /// @brief Initializes Dear ImGui's Vulkan and GLFW platform backends.
    /// @param window GLFW window used by the ImGui platform backend.
    void Init(GLFWwindow* window) override;

    /// @brief Shuts down Dear ImGui's Vulkan and GLFW platform backends.
    void Shutdown() override;

    /// @brief Begins a new frame for both Vulkan and GLFW.
    void NewFrame() override;

    /// @brief Records ImGui draw data into Nodens' active command buffer.
    /// @param drawData Draw data produced by ImGui::Render().
    void RenderDrawData(ImDrawData* drawData) override;

private:
    VulkanContext& m_Context;
};
} // namespace Nodens
