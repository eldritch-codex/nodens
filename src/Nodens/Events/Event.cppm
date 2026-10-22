/// @file Event.cppm
/// @brief Definition of `concept` used to enforce POD events with a proper `Name` constexpr member
/// variable and type definition for EventTypeID sequential type identification.
/// @details All Event types must be plain, trivially copyable structs with no base class or
/// vtable, and contain a `static constexpr std::string_view Name` member.
/// - Input events are routed through the LayerStack via a std::variant(`RoutedInputEvent`).
/// - Non-Input events bypass the LayerStack entirely and are handled exclusively by the EventBus
/// (see EventBus.cppm).
/// @ingroup EventSystem

export module Nodens.Event;
import std;

export namespace Nodens
{

// ─────────────────────────────────────────────────────────────────────────────
// Event Concept
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Requires event types to be trivially copyable, standard-layout structs with a proper
/// 'Name'
///  member variable.
/// @details This ensures events can be safely memcpy'd into contiguous byte buffers and carry a
/// compile-time name for debug logging and identification.
/// @code
/// // Example event struct
/// struct ExampleEvent {
///     static constexpr std::string_view Name = "ExampleEvent";
/// }
/// @endcode
template <typename TEvent>
concept Event =
    std::is_standard_layout_v<TEvent> && std::is_trivially_copyable_v<TEvent> && requires {
        { TEvent::Name } -> std::same_as<const std::string_view&>;
    };

// ─────────────────────────────────────────────────────────────────────────────
// Event Type Identification (Sequential Atomic ID counter)
// ─────────────────────────────────────────────────────────────────────────────

using EventTypeID = std::uint64_t;

namespace Detail
{

/// @brief Global atomic counter for assigning sequential event type IDs.
/// @details Each unique event type receives the next available integer (0, 1, 2, …)
/// on first access. The counter is incremented atomically so registration is
/// thread-safe even when the first EventBus::Subscribe/QueueEvent for a given type
/// races across threads.
inline std::atomic<EventTypeID> g_NextEventTypeId{0};

/// @brief Per-type ID storage. Initialized once via the inline initializer,
/// which atomically fetches-and-increments the global counter.
template <Event TEvent>
inline const EventTypeID g_EventTypeId = g_NextEventTypeId.fetch_add(1);

} // namespace Detail

/// @brief Returns the unique sequential type ID for any Event type.
/// @return A sequential integer unique to T, assigned on first access.
template <Event TEvent>
inline EventTypeID GetEventTypeId()
{
    return Detail::g_EventTypeId<TEvent>;
}

} // namespace Nodens
