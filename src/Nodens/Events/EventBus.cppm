/// @file EventBus.cppm
/// @brief Type-safe publish/subscribe event bus with immediate and queued dispatch.
/// @details The EventBus handles non-input events (subsystems, user-defined)
/// that are decoupled from the LayerStack. It provides:
/// - **Immediate dispatch**: immediate handler invocation in the calling thread.
/// - **Queued dispatch**: events are copied into a double-buffered byte array and drained
/// once per frame via Flush().
/// - **Sequential type IDs**: each event type is assigned a unique, sequential integer
/// via an atomic counter on first use. No RTTI, no hashing.
/// @ingroup EventSystem

export module Nodens.EventBus;
import Nodens.Event;
import Nodens.Log;
import std;

export namespace Nodens
{

// ─────────────────────────────────────────────────────────────────────────────
// Subscription Handle
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Opaque handle returned by Subscribe(), used to later Unsubscribe().
using SubscriptionID = std::uint64_t;

// ─────────────────────────────────────────────────────────────────────────────
// EventBus
// ─────────────────────────────────────────────────────────────────────────────

/// @brief A publish/subscribe event bus for decoupled, type-safe event routing.
/// @details Subscribers register typed handlers via Subscribe<T>().
/// Events are dispatched either synchronously (via `Dispatch()`) or deferred into a double-buffered
/// byte array (via `QueueEvent()`) and drained once per frame via `Flush()`. This bus is intended
/// for non-input events. Input events are routed through the LayerStack instead (see
/// `Layer::OnInputEvent`).
/// @note Thread-safe. Subscribe, Unsubscribe, and QueueEvent may be called
/// from any thread. Dispatch and Flush should be called from the main
/// thread.
/// @see Event, GetEventTypeId
/// @ingroup Events
class EventBus
{
public:
    EventBus() = default;

    /// @brief Subscribes a typed event handler for events of type TEvent.
    /// @details Stores a type-erased wrapper for the event handler into the vector.
    /// @param handler The callback to invoke when an event of type TEvent is dispatched.
    /// @return A SubscriptionId that can be passed to `Unsubscribe()` later.
    template <Event TEvent, typename THandler>
        requires std::invocable<THandler, TEvent&>
    [[nodiscard]]
    SubscriptionID Subscribe(THandler&& handler)
    {
        const auto typeId = GetEventTypeId<TEvent>();

        // Thread-safe guarantee that this event type's dispatch function is registered.
        static std::once_flag wasTypeRegistered{};
        std::call_once(wasTypeRegistered, [this]() { RegisterEventType<TEvent>(); });

        const auto subscriptionId = m_NextId.fetch_add(1);
        const auto typeErasedWrapper = [h = std::forward<THandler>(handler)](void* data)
        { h(*static_cast<TEvent*>(data)); };

        m_Subscribers[typeId].emplace_back(subscriptionId, std::move(typeErasedWrapper));

        CoreLogger().info("EventBus: New subscription (id={}) to Event '{}' (typeId={})",
                          subscriptionId,
                          TEvent::Name,
                          typeId);
        return subscriptionId;
    }

    /// @brief Removes a previously registered subscription.
    /// @param id The SubscriptionId returned by Subscribe().
    void Unsubscribe(SubscriptionID id);

    /// @brief Dispatches an event synchronously to all registered subscribers.
    /// @param event The event to dispatch. Passed by reference to all handlers.
    /// @details Handlers are invoked sequentially in the calling thread, in
    /// subscription order.
    template <Event TEvent>
    void Dispatch(TEvent& event)
    {
        CoreLogger().trace("EventBus: Synchronous dispatch of Event '{}' (typeId={})",
                           TEvent::Name,
                           GetEventTypeId<TEvent>());
        DispatchToSubscribers(GetEventTypeId<TEvent>(), &event);
    }

    /// @brief Queues an event for deferred dispatch during the next Flush() call.
    /// @param event The event value to queue. Copied into a contiguous byte buffer.
    /// @details This call returns immediately. The event is stored in a pre-allocated write buffer.
    /// During Flush(), the buffers are swapped and queued events are dispatched synchronously.
    /// @note Thread-safe. May be called from any thread.
    template <Event TEvent>
    void QueueEvent(TEvent event)
    {
        const auto typeId = GetEventTypeId<TEvent>();

        // Thread-safe guarantee that this event type's dispatch function is registered.
        static std::once_flag wasTypeRegistered{};
        std::call_once(wasTypeRegistered, [this]() { RegisterEventType<TEvent>(); });

        constexpr auto dataSize = sizeof(TEvent);

        std::lock_guard lock(m_BufferMutex);
        auto& activeBuffer = m_Buffers[m_ActiveBufferIndex][typeId];
        auto offset = activeBuffer.size();
        activeBuffer.resize(offset + dataSize);
        std::memcpy(activeBuffer.data() + offset, &event, dataSize);

        CoreLogger().trace("EventBus: Queued '{}' ({} bytes) [{} subscribers]",
                           TEvent::Name,
                           dataSize,
                           m_Subscribers[typeId].size());
    }

    /// @brief Check if an event type is registered.
    /// @note EventTypeIDs start at 0.
    inline bool IsEventTypeRegistered(EventTypeID typeId)
    {
        return typeId < m_DispatchTable.size();
    }

    /// @brief Drains the async event queue, dispatching all deferred events.
    /// @details Swaps the double buffers, then iterates the read buffer.
    /// For each entry, the type ID is used to look up a dispatch function that
    /// deserializes the event data and invokes Dispatch<T>().
    /// @note Must be called from the main thread, typically once per frame.
    void Flush();

private:
    /// @brief Type erased function used to wrap Event subscribers' handlers.
    /// @todo (perf) Define a custom Delegate to replace std::function and avoid heap allocations.
    using TypeErasedWrapperFn = std::function<void(void*)>;

    /// @brief Dispatcher used to multicast events to all subscribers of its type.
    using DispatchFn = std::function<void(std::byte*, std::size_t)>;

    /// @brief Internal subscriber entry: ID + type-erased wrapper.
    struct SubscriberEntry
    {
        SubscriptionID Id;
        TypeErasedWrapperFn Handler;
    };

    /// @brief Dispatches an event to all subscribers of a given type.
    /// @param typeId The sequential event type ID.
    /// @param eventData Pointer to the event data (cast to the concrete type by
    /// each handler).
    /// @details Copies the handler list before invoking, so handlers may safely
    /// call Subscribe or Unsubscribe without deadlocking.
    void DispatchToSubscribers(EventTypeID typeId, void* eventData);

    /// @brief Registers a new event type by growing storage and generating its dispatcher.
    /// @details All events are registered to a multicast dispatcher lambda that is responsible for
    /// 1. rebuilding the event data from the byte array stored in the buffer,
    /// 2. calling `DispatchToSubscribers()` for that event.
    /// @note Caller must hold m_RegistryMutex.
    template <Event TEvent>
    void RegisterEventType()
    {
        const auto typeId = GetEventTypeId<TEvent>();

        if (IsEventTypeRegistered(typeId))
            return;

        auto newRegisteredEventsCount = typeId + 1;

        {
            std::lock_guard lock(m_BufferMutex);
            m_Buffers[0].resize(newRegisteredEventsCount);
            m_Buffers[1].resize(newRegisteredEventsCount);
        }
        {
            std::lock_guard lock(m_RegistryMutex);
            m_Subscribers.resize(newRegisteredEventsCount);
            m_DispatchTable.resize(newRegisteredEventsCount);
            m_RegisteredEventsNames.resize(newRegisteredEventsCount);

            m_RegisteredEventsNames[typeId] = TEvent::Name;
            m_DispatchTable[typeId] = [this, typeId](std::byte* data, std::size_t size)
            {
                constexpr auto eventTypeSize = sizeof(TEvent);
                std::size_t count = size / eventTypeSize;
                for (std::size_t i = 0; i < count; ++i)
                {
                    TEvent event;
                    std::memcpy(&event, data + i * eventTypeSize, eventTypeSize);
                    DispatchToSubscribers(typeId, &event);
                }
            };
        }
        CoreLogger().info("EventBus: Registered Event '{}' (typeId={}).", TEvent::Name, typeId);
    }

private:
    /// @brief Default capacity for each async buffer (64 KB).
    static constexpr std::size_t kDefaultBufferCapacity{64 * 1024};

    /// @brief Monotonically increasing subscription ID counter.
    std::atomic<SubscriptionID> m_NextId{};

    /// @brief Subscriber lists indexed by sequential event type ID.
    std::vector<std::vector<SubscriberEntry>> m_Subscribers;

    /// @brief Type-erased dispatch functions indexed by sequential event type ID.
    std::vector<DispatchFn> m_DispatchTable;

    ///@brief Registered events names indexed by EventTypeID.
    std::vector<std::string_view> m_RegisteredEventsNames;

    /// @brief Mutex protecting m_Subscribers, m_DispatchTable and m_RegisteredEventName.
    std::mutex m_RegistryMutex;

    /// @brief Per-type double-buffered byte arrays for async event storage.
    std::vector<std::vector<std::byte>> m_Buffers[2];

    /// @brief Index of the current write buffer (0 or 1).
    std::size_t m_ActiveBufferIndex{};

    /// @brief Mutex protecting the write buffers and write index.
    std::mutex m_BufferMutex;
};

} // namespace Nodens
