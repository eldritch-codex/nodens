/// @file OpenGLImGuiRenderer.cppm
/// @brief OpenGL + GLFW implementation of the ImGuiRenderer interface.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>
#include <imgui.h>

export module Nodens.OpenGLImGuiRenderer;

import Nodens.ImGuiRenderer;

export namespace Nodens
{
/// @brief Concrete ImGuiRenderer implementation using the OpenGL3 and GLFW ImGui backends.
/// @details Wraps the `imgui_impl_opengl3` and `imgui_impl_glfw` backend functions,
///          isolating their headers to this translation unit. This is the default renderer
///          used by ImGuiLayer when the application is running on an OpenGL context.
/// @see ImGuiRenderer, ImGuiLayer
/// @ingroup Platform
class OpenGLImGuiRenderer : public ImGuiRenderer
{
public:
    OpenGLImGuiRenderer() = default;
    ~OpenGLImGuiRenderer() override = default;

    /// @brief Initializes the OpenGL3 and GLFW ImGui backends for the given window.
    /// @param window The GLFW window handle to bind to.
    void Init(GLFWwindow* window) override;

    /// @brief Shuts down the OpenGL3 and GLFW ImGui backends.
    void Shutdown() override;

    /// @brief Begins a new frame for both the OpenGL3 and GLFW backends.
    void NewFrame() override;

    /// @brief Submits the ImGui draw data to OpenGL for rendering.
    /// @param drawData The draw data produced by ImGui::Render().
    void RenderDrawData(ImDrawData* drawData) override;
};
} // namespace Nodens
