/// @file ImGuiLayer.cppm
/// @brief Overlay layer that manages the Dear ImGui lifecycle within the Nodens application.
/// @ingroup ImGui

export module Nodens.ImGuiLayer;

import Nodens.ImGuiRenderer;
import Nodens.Layer;
import Nodens.TimeStep;
import Nodens.DefaultTheme;
import Nodens.InputEvents;
import std;

export namespace Nodens
{
/// @brief A special overlay Layer that owns the ImGui context and delegates rendering
///        to an injected ImGuiRenderer backend.
/// @details ImGuiLayer is automatically pushed as an overlay by Application when
///          `EnableGUI` is true. It creates the ImGui, ImPlot, and ImPlot3D contexts
///          on attach, applies the selected DefaultTheme, and brackets each frame with
///          Begin() / End() calls so that other layers can issue ImGui draw commands
///          in their OnImGuiRender() overrides.
///
///          By default, ImGuiLayer blocks keyboard and mouse events from propagating
///          to lower layers when ImGui wants to capture them.
/// @see ImGuiRenderer, ILayer, DefaultTheme
/// @ingroup ImGui
class ImGuiLayer : public ILayer
{
public:
    /// @brief Constructs the ImGui layer with a rendering backend and a visual theme.
    /// @param renderer Shared pointer to the concrete ImGuiRenderer implementation.
    /// @param theme    The default visual theme to apply on attach.
    ImGuiLayer(const std::shared_ptr<ImGuiRenderer>& renderer, EDefaultTheme theme);

    ~ImGuiLayer();

    /// @brief Initializes ImGui, ImPlot, and ImPlot3D contexts, applies the theme, and
    ///        initializes the rendering backend.
    void OnAttach() override;

    /// @brief Shuts down the rendering backend and destroys the ImGui contexts.
    void OnDetach() override;

    /// @brief Reserved for future internal ImGui rendering. Currently a no-op.
    /// @param ts The frame delta time.
    void OnImGuiRender(TimeStep ts) override;

    /// @brief Optionally consumes mouse/keyboard events when ImGui wants input capture.
    /// @param e The event to inspect. May be marked as handled.
    void OnInputEvent(RoutedInputEvent& e) override;

    /// @brief Begins a new ImGui frame. Call this before layers issue draw commands.
    /// @details Delegates NewFrame() to the renderer, then starts the ImGui frame
    ///          and sets up a fullscreen dockspace.
    void Begin();

    /// @brief Ends the ImGui frame: renders draw data and updates platform windows.
    void End();

    /// @brief Enables or disables event blocking when ImGui has input focus.
    /// @param block If true, mouse and keyboard events are consumed when ImGui wants them.
    void BlockEvents(bool block)
    {
        m_BlockInputEvents = block;
    }

private:
    bool m_BlockInputEvents = true;              ///< Whether to consume events captured by ImGui.
    EDefaultTheme m_Theme = EDefaultTheme::Dark; ///< The visual theme applied during OnAttach().
    std::shared_ptr<ImGuiRenderer> m_Renderer;   ///< The injected rendering backend.
};
} // namespace Nodens
