/// @file OpenGLContext.cpp
/// @brief Implementation of OpenGLContext: GLAD initialization, GL state setup, and buffer swap.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>
#include <glad/glad.h>

module Nodens.OpenGLContext;

import Nodens.Log;

import Nodens.Log;

namespace Nodens
{

OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
{
    if (!windowHandle)
        FatalCore("Window handle is null!");
}

void OpenGLContext::Init()
{
    glfwMakeContextCurrent(m_WindowHandle);
    // glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!status)
        FatalCore("Failed to initialize glad!");

    CoreLogger().info("OpenGL Info:");
    CoreLogger().info("  Version: {}", (const char*)glGetString(GL_VERSION));
    CoreLogger().info("  GPU Used: {}", (const char*)glGetString(GL_RENDERER));

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLContext::Present()
{
    glfwSwapBuffers(m_WindowHandle);
}

} // namespace Nodens
