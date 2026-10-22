/// @file InputEvents.cppm
/// @brief Definition of input events.
/// @ingroup EventSystem

export module Nodens.InputEvents;
import Nodens.Event;
import Nodens.Log;
import std;

export namespace Nodens::InputEvents
{
// ─────────────────────────────────────────────────────────────────────────────
// Keyboard Events
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Event triggered when a keyboard key is pressed down.
/// @ingroup InputEvents
struct KeyPressed
{
    static constexpr std::string_view Name = "KeyPressed";
    int KeyCode;     ///< The key being pressed.
    int RepeatCount; ///< 0 for initial press, >0 if the key is held (OS
                     ///< auto-repeat).
};
static_assert(Event<KeyPressed>);

/// @brief Event triggered when a keyboard key is released.
/// @ingroup InputEvents
struct KeyReleased
{
    static constexpr std::string_view Name = "KeyReleased";
    int KeyCode; ///< The key being released.
};
static_assert(Event<KeyReleased>);

/// @brief Event triggered for text input (character input handling).
/// @ingroup InputEvents
struct KeyTyped
{
    static constexpr std::string_view Name = "KeyTyped";
    int KeyCode; ///< The character code.
};
static_assert(Event<KeyTyped>);

// ─────────────────────────────────────────────────────────────────────────────
// Mouse Events
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Event triggered when the mouse cursor moves.
/// @ingroup InputEvents
struct MouseMoved
{
    static constexpr std::string_view Name = "MouseMoved";
    float X; ///< Absolute X coordinate relative to the window.
    float Y; ///< Absolute Y coordinate relative to the window.
};
static_assert(Event<MouseMoved>);

/// @brief Event triggered by the mouse scroll wheel.
/// @ingroup InputEvents
struct MouseScrolled
{
    static constexpr std::string_view Name = "MouseScrolled";
    float XOffset; ///< Horizontal scroll amount (usually 0).
    float YOffset; ///< Vertical scroll amount (wheel delta).
};
static_assert(Event<InputEvents::MouseScrolled>);

/// @brief Event triggered when a mouse button is pressed.
/// @ingroup InputEvents
struct MouseButtonPressed
{
    static constexpr std::string_view Name = "MouseButtonPressed";
    int Button; ///< The mouse button code.
};
static_assert(Event<InputEvents::MouseButtonPressed>);

/// @brief Event triggered when a mouse button is released.
/// @ingroup InputEvents
struct MouseButtonReleased
{
    static constexpr std::string_view Name;
    int Button; ///< The mouse button code.
};
static_assert(Event<InputEvents::MouseButtonReleased>);

// ─────────────────────────────────────────────────────────────────────────────
// Window Events
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Event triggered when the native window is resized.
/// @ingroup InputEvents
struct WindowResize
{
    static constexpr std::string_view Name = "WindowResize";
    unsigned int Width;  ///< New width in pixels.
    unsigned int Height; ///< New height in pixels.
};
static_assert(Event<InputEvents::WindowResize>);

/// @brief Event triggered when the user attempts to close the window.
/// @ingroup InputEvents
struct WindowClose
{
    static constexpr std::string_view Name = "WindowClose";
};
static_assert(Event<InputEvents::WindowClose>);
} // namespace Nodens::InputEvents

export namespace Nodens
{
// ─────────────────────────────────────────────────────────────────────────────
// InputEvent Variant (LayerStack-Routed)
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Closed set of input events routed through the LayerStack.
/// @details These events originate from platform callbacks (GLFW) and propagate
///          back-to-front through the layer stack with handled-flag semantics.
///          Non-input events do NOT use this - they use the EventBus.
using InputEvent = std::variant<InputEvents::KeyPressed,
                                InputEvents::KeyReleased,
                                InputEvents::KeyTyped,
                                InputEvents::MouseMoved,
                                InputEvents::MouseScrolled,
                                InputEvents::MouseButtonPressed,
                                InputEvents::MouseButtonReleased,
                                InputEvents::WindowResize,
                                InputEvents::WindowClose>;

/// @brief Wrapper carrying the variant and a handled flag for LayerStack
/// propagation.
/// @details Passed by reference through
/// `Layer::OnInputEvent(RoutedInputEvent&)`.
///          Set `Handled = true` to consume the event and stop back-to-front
///          propagation.
struct RoutedInputEvent
{
    InputEvent Event;    ///< The concrete event data.
    bool Handled{false}; ///< If true, lower layers will not receive this event.
};

// ─────────────────────────────────────────────────────────────────────────────
// InputEvent Dispatcher
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Utility class for dispatching InputEvents based on their variant
/// type.
/// @code(cpp)
///     InputEventDispatcher dispatcher(eventData);
///     dispatcher.Dispatch<WindowCloseEvent>([](WindowCloseEvent& e) { return true; });
///     dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) { ... });
/// @endcode
class InputEventDispatcher
{
public:
    /// @brief Constructs a dispatcher bound to an RoutedInputEvent reference.
    /// @param data The input event data to dispatch.
    InputEventDispatcher(RoutedInputEvent& routedEvent) : m_RoutedEvent(routedEvent)
    {
    }

    /// @brief Dispatches the event to a handler if the variant holds type TEvent.
    /// @tparam TEvent The concrete event type to match against.
    /// @tparam THandler A callable taking TEvent& and returning a value convertible to bool.
    /// @param func The handler function. Return true to mark the event as handled.
    /// @return True if the variant held type T and the handler was invoked.
    template <Event TEvent, typename THandler>
        requires std::invocable<THandler, TEvent&>
    bool Dispatch(THandler&& handler)
    {
        if (auto* ev = std::get_if<TEvent>(&m_RoutedEvent.Event))
        {
            m_RoutedEvent.Handled |=
                static_cast<bool>(std::invoke(std::forward<THandler>(handler), *ev));

            CoreLogger().trace("InputEventDispatcher: Matched '{}', handled={}.",
                               TEvent::Name,
                               m_RoutedEvent.Handled);
            return true;
        }
        return false;
    }

private:
    RoutedInputEvent& m_RoutedEvent;
};

// ─────────────────────────────────────────────────────────────────────────────
// InputEvent Type Query Helpers
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Checks if the input event is a mouse-related type.
/// @param e The input event variant.
/// @return True if the event is a mouse movement, scroll, or button event.
constexpr bool IsMouseEvent(const InputEvent& e)
{
    return std::holds_alternative<InputEvents::MouseMoved>(e) ||
           std::holds_alternative<InputEvents::MouseScrolled>(e) ||
           std::holds_alternative<InputEvents::MouseButtonPressed>(e) ||
           std::holds_alternative<InputEvents::MouseButtonReleased>(e);
}

/// @brief Checks if the input event is a keyboard-related type.
/// @param e The input event variant.
/// @return True if the event is a key press, release, or typed event.
constexpr bool IsKeyboardEvent(const InputEvent& e)
{
    return std::holds_alternative<InputEvents::KeyPressed>(e) ||
           std::holds_alternative<InputEvents::KeyReleased>(e) ||
           std::holds_alternative<InputEvents::KeyTyped>(e);
}

/// @brief Retrieves the static name string from whichever event type is held.
/// @param e The input event variant.
/// @return The compile-time name of the active event type.
inline std::string_view GetInputEventName(const InputEvent& e)
{
    return std::visit([](const auto& event) -> std::string_view { return event.Name; }, e);
}

} // namespace Nodens

// ─────────────────────────────────────────────────────────────────────────────
// Formatters for Nodens event types
// ─────────────────────────────────────────────────────────────────────────────

template <>
struct std::formatter<Nodens::InputEvents::KeyPressed> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::KeyPressed& e, FormatContext& ctx) const
    {
        return std::format_to(
            ctx.out(), "KeyPressedEvent: {} ({} repeats)", e.KeyCode, e.RepeatCount);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::KeyReleased> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::KeyReleased& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "KeyReleasedEvent: {}", e.KeyCode);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::KeyTyped> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::KeyTyped& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "KeyTypedEvent: {}", e.KeyCode);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::MouseMoved> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::MouseMoved& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "MouseMovedEvent: {}, {}", e.X, e.Y);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::MouseScrolled> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::MouseScrolled& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "MouseScrolledEvent: {}, {}", e.XOffset, e.YOffset);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::MouseButtonPressed> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::MouseButtonPressed& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "MouseButtonPressedEvent: {}", e.Button);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::MouseButtonReleased> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::MouseButtonReleased& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "MouseButtonReleasedEvent: {}", e.Button);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::WindowResize> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::WindowResize& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "WindowResizeEvent: {}, {}", e.Width, e.Height);
    }
};

template <>
struct std::formatter<Nodens::InputEvents::WindowClose> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::InputEvents::WindowClose&, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "WindowCloseEvent");
    }
};

template <>
struct std::formatter<Nodens::RoutedInputEvent> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Nodens::RoutedInputEvent& e, FormatContext& ctx) const
    {
        return std::visit([&ctx](const auto& ev) { return std::format_to(ctx.out(), "{}", ev); },
                          e.Event);
    }
};
