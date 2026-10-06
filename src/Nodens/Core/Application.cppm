/// @file Application.cppm
/// @brief Core application class and startup configuration for the Nodens framework.
/// @ingroup Core

export module Nodens.Application;

import Nodens.EventBus;
import Nodens.InputEvents;
import Nodens.JobSystem;
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
struct FApplicationSpecification
{
    std::string Name{"Nodens Application"}; ///< Window title and application identifier.
    std::uint32_t WindowWidth{1280};        ///< Initial window width in pixels.
    std::uint32_t WindowHeight{720};        ///< Initial window height in pixels.
    bool EnableGUI{true};                   ///< Whether to create the ImGui overlay layer.
    bool VSync{false};                      ///< If true, vertical synchronization is enabled.
    EGraphicsAPI GraphicsAPI{
        EGraphicsAPI::Vulkan}; ///< Client graphics API for the application window.
    bool ShouldImGuiBlockInputs{true};
    EDefaultTheme DefaultTheme{EDefaultTheme::Dark};
};

/// @brief The central singleton that owns the window, layer stack, job system, and main loop.
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
/// @see ILayer, LayerStack, FApplicationSpecification
/// @ingroup Core
class Application
{
public:
    /// @brief Constructs the application, creating the window, job system, and optional ImGui
    /// layer.
    /// @param specification The configuration to use for initialization.
    explicit Application(const FApplicationSpecification& specification);

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

    /// @brief Returns a reference to the application's job system.
    /// @return Reference to the JobSystem instance.
    JobSystem& GetJobSystem();

    /// @brief Returns a reference to the application's event bus.
    /// @reutnr Reference to EventBus instance.
    EventBus& GetEventBus();

    /// @brief Returns the specification used to initialize this application.
    /// @return Const reference to the FApplicationSpecification.
    const FApplicationSpecification& GetSpecification() const;

    /// @brief Retrieves the global singleton Application instance.
    /// @return Reference to the running Application.
    static Application& Get();

private:
    /// @brief Handles the WindowCloseEvent by setting the running flag to false.
    /// @param e The window close event.
    /// @return Always returns true (event is consumed).
    bool OnWindowClose(InputEvents::WindowClose& e);

    FApplicationSpecification m_Specification; ///< Stored copy of the startup configuration.
    bool m_Running{true};                      ///< Main loop sentinel; false triggers shutdown.

    std::unique_ptr<IWindow> m_Window; ///< The platform window.

    ImGuiLayer* m_ImGuiLayer{nullptr}; ///< The ImGui overlay (owned by LayerStack).

    std::unique_ptr<LayerStack> m_LayerStack; ///< Ordered collection of active layers.
    std::unique_ptr<JobSystem> m_JobSystem;   ///< The multithreaded job system.
    std::unique_ptr<EventBus> m_EventBus;     ///< The event bus system.

    float m_LastFrameTime{0.0f}; ///< Timestamp of the previous frame (seconds since start).

private:
    static Application* s_Instance; ///< Pointer to the sole Application instance.
};

} // namespace Nodens
