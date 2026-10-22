/// @file ImGuiRenderer.cppm
/// @brief Abstract rendering backend interface for Dear ImGui.
/// @details Concrete implementations (e.g., OpenGLImGuiRenderer) provide the platform-specific
///          calls for initializing, shutting down, and rendering ImGui draw data.
/// @ingroup ImGui

module;

#include <GLFW/glfw3.h>
#include <imgui.h>

export module Nodens.ImGuiRenderer;

export namespace Nodens
{
/// @brief Abstract interface that decouples ImGui rendering from a specific graphics API.
/// @details The ImGuiLayer delegates all backend-specific operations (initialization, frame
///          start, and draw-data submission) to an ImGuiRenderer implementation. This allows
///          swapping rendering backends (OpenGL, Vulkan, etc.) without modifying the layer.
/// @see ImGuiLayer, OpenGLImGuiRenderer
/// @ingroup ImGui
class ImGuiRenderer
{
public:
    virtual ~ImGuiRenderer() = default;

    /// @brief Initializes the rendering backend for the given GLFW window.
    /// @param window The native GLFW window handle to bind to.
    virtual void Init(GLFWwindow* window) = 0;

    /// @brief Tears down the rendering backend and releases GPU resources.
    virtual void Shutdown() = 0;

    /// @brief Begins a new ImGui frame on the backend side.
    /// @details Called once per frame before ImGui::NewFrame().
    virtual void NewFrame() = 0;

    /// @brief Submits ImGui draw data to the GPU for rendering.
    /// @param drawData The draw data produced by ImGui::Render().
    virtual void RenderDrawData(ImDrawData* drawData) = 0;
};
} // namespace Nodens
