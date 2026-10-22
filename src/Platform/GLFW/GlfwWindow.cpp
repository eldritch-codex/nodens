/// @file GlfwWindow.cpp
/// @brief GLFW implementation of the Window interface.
/// @details Contains the GlfwWindow class which manages GLFW window creation, event
///          callback registration, and per-frame update. Also provides the factory
///          implementation for IWindow::Create().
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>

module Nodens.Window;

import Nodens.GraphicsContext;
import Nodens.Log;
#if defined(ND_HAS_OPENGL)
import Nodens.OpenGLContext;
#endif
#if defined(ND_HAS_VULKAN)
import Nodens.VulkanContext;
#endif
import std;

namespace Nodens
{

/// @brief Concrete GLFW-backed Window implementation.
/// @details Owns the GLFWwindow handle, an optional OpenGLContext, and a WindowData
///          struct that stores dimensions, VSync state, and the event callback. GLFW callbacks are
///          registered during Init() and dispatch Nodens Event objects to the application.
/// @ingroup Platform
class GLFWWindow : public IWindow
{
public:
    /// @brief Constructs and initializes a GLFW window with the given properties.
    /// @param props Window configuration (title, dimensions, VSync).
    GLFWWindow(const FWindowProps& props)
    {
        Init(props);
    }

    ~GLFWWindow() override
    {
        Shutdown();
    }

    /// @brief Polls GLFW events and swaps OpenGL buffers when an OpenGL context exists.
    void OnUpdate() override
    {
        ZoneScoped;

        glfwPollEvents();
        if (m_Context)
            m_Context->Present();
    }

    unsigned int GetWidth() const override
    {
        return m_Data.Width;
    }

    unsigned int GetHeight() const override
    {
        return m_Data.Height;
    }

    /// @brief Sets the event callback function invoked on window/input events.
    /// @param callback The function to receive Event references.
    void SetInputEventCallback(const InputEventCallbackFn& callback) override
    {
        m_Data.InputEventCallback = callback;
    }

    /// @brief Enables or disables VSync for an OpenGL context.
    /// @param enabled True to enable VSync (swap interval 1), false to disable (swap interval 0).
    void SetVSync(bool enabled) override
    {
        if (m_Data.API == EGraphicsAPI::OpenGL)
        {
            if (enabled)
                glfwSwapInterval(1);
            else
                glfwSwapInterval(0);
        }

        m_Data.VSync = enabled;
    }

    bool IsVSyncOn() const override
    {
        return m_Data.VSync;
    }

    /// @brief Returns the raw GLFWwindow pointer.
    /// @return Opaque pointer (cast to `GLFWwindow*` by the caller).
    void* GetNativeWindow() const override
    {
        return m_Window;
    }

    /// @brief Returns the graphics context owned by this window.
    /// @return Borrowed graphics context, or null when no context exists.
    IGraphicsContext* GetGraphicsContext() const override
    {
        return m_Context;
    }

private:
    /// @brief Initializes GLFW (if needed), creates the window, and registers all callbacks.
    /// @param props Window configuration.
    void Init(const FWindowProps& props);

    /// @brief Destroys the GLFW window.
    void Shutdown();

    GLFWwindow* m_Window = nullptr;        ///< The native GLFW window handle.
    IGraphicsContext* m_Context = nullptr; ///< The backend context bound to this window.

    /// @brief Internal data bundle attached to the GLFW window via glfwSetWindowUserPointer.
    /// @details GLFW callbacks retrieve this struct to dispatch Nodens events and update
    ///          cached dimensions.
    struct WindowData
    {
        std::string Title;   ///< Current window title.
        unsigned int Width;  ///< Current width in pixels.
        unsigned int Height; ///< Current height in pixels.
        bool VSync;          ///< Whether VSync is enabled.
        EGraphicsAPI API;    ///< The graphics API to use for rendering.

        InputEventCallbackFn InputEventCallback; ///< The application's event callback.
    };

    WindowData m_Data; ///< Cached window state accessible from GLFW callbacks.
};

/// @brief Tracks whether GLFW has been initialized (ensures glfwInit is called only once).
static bool s_GLFWInitialized = false;

/// @brief GLFW error callback that logs errors through the Nodens core logger.
/// @param error GLFW error code.
/// @param description Human-readable error description.
static void GLFWErrorCallback(int error, const char* description)
{
    CoreLogger().error("GLFW Error ({}): {}", error, description);
}

/// @brief Factory implementation: creates a GlfwWindow.
/// @param props Window configuration.
/// @return A new GLFWWindow (caller takes ownership).
IWindow* IWindow::Create(const FWindowProps& props)
{
    return new GLFWWindow(props);
}

/// @brief Initializes GLFW, creates the window, optionally sets up OpenGL, and registers
///        all GLFW event callbacks (resize, close, key, mouse button, scroll, cursor).
void GLFWWindow::Init(const FWindowProps& props)
{
    ZoneScoped;

    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;
    m_Data.VSync = props.VSync;
    m_Data.API = props.API;

    CoreLogger().info("Creating window {} ({}, {})", props.Title, props.Width, props.Height);

    if (!s_GLFWInitialized)
    {
        int succes = glfwInit();
        if (!succes)
            FatalCore("Could not initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);

        s_GLFWInitialized = true;
    }

    if (props.API == EGraphicsAPI::OpenGL)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
    else
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    m_Window = glfwCreateWindow(
        (int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

    if (props.API == EGraphicsAPI::OpenGL)
    {
#if defined(ND_HAS_OPENGL)
        m_Context = new OpenGLContext(m_Window);
#else
        FatalCore("Nodens was built without OpenGL support!");
#endif
    }
    else if (props.API == EGraphicsAPI::Vulkan)
    {
#if defined(ND_HAS_VULKAN)
        m_Context = new VulkanContext(m_Window);
#else
        FatalCore("Nodens was built without Vulkan support!");
#endif
    }

    if (m_Context)
        m_Context->Init();

    glfwSetWindowUserPointer(m_Window, &m_Data);
    // This function assigns the WindowData struct to the GLFWwindow object.
    // This allows us to retrieve the WindowData from the GLFWwindow* by
    // using the function glfwGetWindowUserPointer(GLFWwindow*) and casting
    // it to WindowData.

    SetVSync(props.VSync);

    //----------------------------------------------------------------------------
    // Set GLFW callbacks
    glfwSetWindowSizeCallback(m_Window,
                              [](GLFWwindow* window, int width, int height)
                              {
                                  auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
                                  data.Width = width;
                                  data.Height = height;

                                  RoutedInputEvent eventData{.Event = InputEvents::WindowResize{
                                                                 .Width = (unsigned int)width,
                                                                 .Height = (unsigned int)height}};
                                  data.InputEventCallback(eventData);
                              });

    glfwSetWindowCloseCallback(m_Window,
                               [](GLFWwindow* window)
                               {
                                   auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
                                   RoutedInputEvent eventData{.Event = InputEvents::WindowClose{}};
                                   data.InputEventCallback(eventData);
                               });

    glfwSetKeyCallback(
        m_Window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    RoutedInputEvent eventData{
                        .Event = InputEvents::KeyPressed{.KeyCode = key, .RepeatCount = 0}};
                    data.InputEventCallback(eventData);
                    break;
                }
                case GLFW_RELEASE:
                {
                    RoutedInputEvent eventData{.Event = InputEvents::KeyReleased{.KeyCode = key}};
                    data.InputEventCallback(eventData);
                    break;
                }
                case GLFW_REPEAT:
                {
                    RoutedInputEvent eventData{
                        .Event = InputEvents::KeyPressed{.KeyCode = key, .RepeatCount = 1}};
                    data.InputEventCallback(eventData);
                    break;
                }
            }
        });

    glfwSetMouseButtonCallback(
        m_Window,
        [](GLFWwindow* window, int button, int action, int mods)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    RoutedInputEvent eventData{
                        .Event = InputEvents::MouseButtonPressed{.Button = button}};
                    data.InputEventCallback(eventData);
                    break;
                }
                case GLFW_RELEASE:
                {
                    RoutedInputEvent eventData{
                        .Event = InputEvents::MouseButtonReleased{.Button = button}};
                    data.InputEventCallback(eventData);
                    break;
                }
            }
        });

    glfwSetScrollCallback(m_Window,
                          [](GLFWwindow* window, double xOffset, double yOffset)
                          {
                              auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
                              RoutedInputEvent eventData{
                                  .Event = InputEvents::MouseScrolled{.XOffset = (float)xOffset,
                                                                      .YOffset = (float)yOffset}};
                              data.InputEventCallback(eventData);
                          });

    glfwSetCursorPosCallback(
        m_Window,
        [](GLFWwindow* window, double xPos, double yPos)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
            RoutedInputEvent eventData{
                .Event = InputEvents::MouseMoved{.X = (float)xPos, .Y = (float)yPos}};
            data.InputEventCallback(eventData);
        });
}

/// @brief Destroys the underlying GLFW window.
void GLFWWindow::Shutdown()
{
    delete m_Context;
    m_Context = nullptr;

    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }

    if (s_GLFWInitialized)
    {
        glfwTerminate();
        s_GLFWInitialized = false;
    }
}
} // namespace Nodens
