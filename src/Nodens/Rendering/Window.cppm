/// @file Window.cppm
/// @brief Abstract window interface and creation properties.
/// @ingroup Rendering

export module Nodens.Window;

import Nodens.GraphicsContext;
import Nodens.InputEvents;
import std;

export namespace Nodens
{

/// @brief Client graphics API requested by a platform window.
/// @details `OpenGL` creates an OpenGL context. `NoAPI` and `Vulkan` create a
///          GLFW window without a client API so an external renderer can own setup.
enum class EGraphicsAPI
{
    NoAPI,
    Vulkan,
    OpenGL,
};

/// @brief Configuration properties for creating a Window.
/// @details Passed to the static IWindow::Create() factory. Provides sensible defaults
///          for title, dimensions, and VSync.
/// @ingroup Rendering
struct FWindowProps
{
    std::string Title{"Nodens"};            ///< The window title displayed in the title bar.
    unsigned int Width{1280};               ///< Initial window width in pixels.
    unsigned int Height{720};               ///< Initial window height in pixels.
    bool VSync{false};                      ///< Whether vertical synchronization is enabled.
    EGraphicsAPI API{EGraphicsAPI::Vulkan}; ///< The graphics API to use for rendering.
};

/// @brief Abstract base class for a platform window.
/// @details Provides a platform-agnostic interface for window management, including
///          update polling, dimension queries, VSync control, event callbacks, and
///          access to the underlying native window handle.
///
///          The concrete implementation (e.g., GlfwWindow) is created via the static
///          factory method Create().
/// @see FWindowProps, GlfwWindow
/// @ingroup Rendering
class IWindow
{
public:
    /// @brief Type alias for the event callback function.
    /// @details The Application sets this callback; the window implementation invokes it
    ///          whenever a platform event (resize, close, key, mouse) occurs.
    using InputEventCallbackFn = std::function<void(RoutedInputEvent&)>;

    virtual ~IWindow()
    {
    }

    /// @brief Polls platform events and presents a frame when the backend supports it.
    virtual void OnUpdate() = 0;

    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    /// @brief Sets the event callback invoked by the window on platform events.
    /// @param callback The function to call with each Event.
    virtual void SetInputEventCallback(const InputEventCallbackFn& callback) = 0;

    /// @brief Enables or disables vertical synchronization.
    /// @param enabled True to enable VSync, false to disable.
    virtual void SetVSync(bool enabled) = 0;

    /// @brief Queries whether VSync is currently enabled.
    /// @return True if VSync is on.
    virtual bool IsVSyncOn() const = 0;

    /// @brief Returns a raw pointer to the underlying native window handle.
    /// @details For GLFW this returns a borrowed `GLFWwindow*`. Cast the result to
    ///          the appropriate type. Nodens owns the handle and keeps it valid until
    ///          the window is destroyed.
    /// @return Borrowed opaque pointer to the native window.
    virtual void* GetNativeWindow() const = 0;

    /// @brief Returns the graphics context owned by the window.
    /// @details The returned pointer is borrowed and remains valid until this window is destroyed.
    ///          It may be null when the window uses `NoAPI` or when no context was created.
    /// @return Borrowed graphics context, or null when no context exists.
    virtual IGraphicsContext* GetGraphicsContext() const = 0;

    /// @brief Static factory method that creates a platform-specific Window.
    /// @param props The configuration properties for the new window.
    /// @return A raw pointer to the newly created IWindow. Caller takes ownership.
    static IWindow* Create(const FWindowProps& props = FWindowProps());
};

} // namespace Nodens
