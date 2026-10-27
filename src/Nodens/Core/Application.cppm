/// @file Application.cppm
/// @brief Core application class and startup configuration for the Nodens framework.
/// @ingroup Core

export module Nodens.Application;

import Nodens.EventBus;
import Nodens.InputEvents;
import Nodens.TaskSystem;
import Nodens.Layer;
import Nodens.LayerStack;
import Nodens.ImGuiLayer;
import Nodens.Window;
import Nodens.DefaultTheme;
import std;

export namespace Nodens
{

/// @brief Configuration data used to initialize an Application instance.
/// @details Provides sensible defaults for a windowed GUI application.
///          Pass a customized instance to the Application constructor to override.
/// @ingroup Core
struct FApplicationConfig
{
    std::string Name{"Nodens Application"}; ///< Window title and application identifier.
    std::uint32_t WindowWidth{1280};        ///< Initial window width in pixels.
    std::uint32_t WindowHeight{720};        ///< Initial window height in pixels.
    bool EnableGUI{true};                   ///< Whether to create the ImGui overlay layer.
    bool VSync{false};                      ///< If true, vertical synchronization is enabled.
    EGraphicsAPI GraphicsAPI{
        EGraphicsAPI::Vulkan}; ///< Client graphics API for the application window.
    bool ShouldImGuiBlockInputs{true};
    ETheme Theme{ETheme::Dark}; ///< The visual theme applied by the ImGui layer.
};

/// @brief The central singleton that owns the window, layer stack, task system, and main loop.
/// @details A client application subclasses Application and pushes its own ILayer instances
///          in the constructor. Exactly one Application instance may exist at any time;
///          creating a second one terminates the process.
///
/// **Typical usage:**
/// @code
/// class MyApp : public Nodens::Application {
/// public:
///     MyApp() : Application({.Name = "Demo"}) {
///         PushLayer(new MyLayer());
///     }
/// };
/// int main() {
///     Nodens::InitializeLoggers();
///     MyApp app;
///     app.Run();
/// }
/// @endcode
///
/// @see ILayer, LayerStack, FApplicationConfig
/// @ingroup Core
class Application
{
public:
    /// @brief Constructs the application, creating the window, task system, and optional ImGui
    /// layer.
    /// @param config The configuration to use for initialization.
    explicit Application(const FApplicationConfig& config);

    virtual ~Application();

    /// @brief Enters the main loop and runs until the window is closed.
    /// @details Each frame updates every layer, renders ImGui, and swaps buffers.
    ///          The loop is driven by a monotonic clock for frame-time calculation.
    void Run();

    /// @brief Dispatches an event through the layer stack (back-to-front).
    /// @param e The event to dispatch. May be marked as handled by a layer.
    void OnInputEvent(RoutedInputEvent& e);

    /// @brief Pushes a regular layer onto the layer stack and calls its OnAttach().
    /// @param layer Raw pointer to the layer. Ownership is transferred to the LayerStack.
    void PushLayer(ILayer* layer);

    /// @brief Pushes an overlay layer (rendered last) and calls its OnAttach().
    /// @param overlay Raw pointer to the overlay. Ownership is transferred to the LayerStack.
    void PushOverlay(ILayer* overlay);

    /// @brief Returns a reference to the application window.
    /// @return Reference to the IWindow instance.
    IWindow& GetWindow();

    /// @brief Returns a reference to the application's task system.
    /// @return Reference to the TaskSystem instance.
    TaskSystem& GetTaskSystem();

    /// @brief Returns a reference to the application's event bus.
    /// @return Reference to EventBus instance.
    EventBus& GetEventBus();

    /// @brief Returns the configuration used to initialize this application.
    /// @return Const reference to the FApplicationConfig.
    const FApplicationConfig& GetConfig() const;

    /// @brief Retrieves the global singleton Application instance.
    /// @return Reference to the running Application.
    static Application& Get();

private:
    /// @brief Handles the WindowCloseEvent by setting the running flag to false.
    /// @param e The window close event.
    /// @return Always false: the close event is deliberately not consumed, so every layer below
    ///         still receives it.
    bool OnWindowClose(InputEvents::WindowClose& e);

    FApplicationConfig m_Config; ///< Stored copy of the startup configuration.
    bool m_Running{true};        ///< Main loop sentinel; false triggers shutdown.

    std::unique_ptr<IWindow> m_Window; ///< The platform window.

    ImGuiLayer* m_ImGuiLayer{nullptr}; ///< The ImGui overlay (owned by LayerStack).

    std::unique_ptr<LayerStack> m_LayerStack; ///< Ordered collection of active layers.
    std::unique_ptr<TaskSystem> m_TaskSystem; ///< The multithreaded task system.
    std::unique_ptr<EventBus> m_EventBus;     ///< The event bus system.

    float m_LastFrameTime{0.0f}; ///< Timestamp of the previous frame (seconds since start).

private:
    static Application* s_Instance; ///< Pointer to the sole Application instance.
};

} // namespace Nodens
