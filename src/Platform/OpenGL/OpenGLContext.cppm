/// @file OpenGLContext.cppm
/// @brief OpenGL implementation of the IGraphicsContext interface.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>

export module Nodens.OpenGLContext;

import Nodens.GraphicsContext;

export namespace Nodens
{
/// @brief Concrete IGraphicsContext implementation for OpenGL via GLFW.
/// @details Creates an OpenGL rendering context on the provided GLFW window, loads
///          function pointers with GLAD, enables default GL state (multisample,
///          depth test, face culling, alpha blending), and handles buffer swapping.
/// @see IGraphicsContext, GlfwWindow
/// @ingroup Platform
class OpenGLContext : public IGraphicsContext
{
public:
    /// @brief Constructs the OpenGL context for a given GLFW window.
    /// @param windowHandle A valid GLFWwindow pointer. Must not be null.
    OpenGLContext(GLFWwindow* windowHandle);

    /// @brief Makes the context current, loads GL functions via GLAD, and sets default GL state.
    void Init() override;

    /// @brief Presents the rendered frame by swapping GLFW buffers.
    void Present() override;

private:
    GLFWwindow* m_WindowHandle; ///< The GLFW window this context is bound to.
};
} // namespace Nodens
