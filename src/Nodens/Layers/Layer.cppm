/// @file Layer.cppm
/// @brief Base class for application layers.
/// @ingroup Core

export module Nodens.Layer;

import Nodens.TimeStep;
import Nodens.InputEvents;
import std;

export namespace Nodens
{
/// @brief Base class representing a single layer in the application's LayerStack.
/// @details Layers are the primary unit of composition in Nodens. Each layer receives lifecycle
/// callbacks for attachment, detachment, per-frame updates, ImGui rendering, and event handling.
/// Override the virtual methods to implement custom behavior.
/// Layers are updated front-to-back but receive events back-to-front, allowing overlays (e.g.,
/// ImGui) to consume input before other layers.
/// @see LayerStack, Application::PushLayer, Application::PushOverlay
/// @ingroup Core
class ILayer
{
public:
    /// @param name A human-readable name used for logging and profiling.
    ILayer(const std::string& name = "Layer");

    virtual ~ILayer() = default;

    /// @brief Called once when the layer is pushed onto the LayerStack.
    /// @details Use this to allocate resources, subscribe to events, etc.
    virtual void OnAttach()
    {
    }

    /// @brief Called once when the layer is removed from the LayerStack.
    /// @details Use this to release resources, unsubscribe from events, etc.
    virtual void OnDetach()
    {
    }

    /// @brief Called every frame with the elapsed time since the last frame.
    /// @param ts The frame delta time.
    virtual void OnUpdate(TimeStep ts)
    {
    }

    /// @brief Called every frame between ImGuiLayer::Begin() and
    /// ImGuiLayer::End().
    /// @details Override this to issue ImGui draw commands for tooling or debug
    /// UI.
    /// @param ts The frame delta time.
    virtual void OnImGuiRender(TimeStep ts)
    {
    }

    /// @brief Called when an input/window event propagates to this layer
    /// (back-to-front order).
    /// @param event The event data wrapper. Set `event.Handled = true` to consume
    /// it.
    virtual void OnInputEvent(RoutedInputEvent& event)
    {
    }

    inline const std::string& GetName() const
    {
        return m_Name;
    }

protected:
    std::string m_Name; ///< Human-readable name for logging and profiling.
};
} // namespace Nodens
