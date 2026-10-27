/// @file OpenGLBackend.cppm
/// @brief OpenGL implementation of the IGraphicsBackend interface.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>

export module Nodens.OpenGLBackend;

import Nodens.GraphicsBackend;

export namespace Nodens
{
/// @brief Concrete IGraphicsBackend implementation for OpenGL via GLFW.
/// @details Creates an OpenGL rendering context on the provided GLFW window, loads
///          function pointers with GLAD, enables default GL state (multisample,
///          depth test, face culling, alpha blending), and handles buffer swapping.
/// @see IGraphicsBackend, GlfwWindow
/// @ingroup Platform
class OpenGLBackend : public IGraphicsBackend
{
public:
    /// @brief Constructs the OpenGL context for a given GLFW window.
    /// @param windowHandle A valid GLFWwindow pointer. Must not be null.
    OpenGLBackend(GLFWwindow* windowHandle);

    /// @brief Makes the context current, loads GL functions via GLAD, and sets default GL state.
    void Init() override;

    /// @brief Presents the rendered frame by swapping GLFW buffers.
    void Present() override;

private:
    GLFWwindow* m_WindowHandle; ///< The GLFW window this context is bound to.
};
} // namespace Nodens
