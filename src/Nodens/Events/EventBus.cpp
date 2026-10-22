/// @file EventBus.cpp
/// @brief Implementation of EventBus non-template methods: constructor,
/// unsubscribe, flush, dispatch.
/// @ingroup EventSystem

module;

#include <tracy/Tracy.hpp>

module Nodens.EventBus;
import Nodens.Event;
import Nodens.Log;
import std;

namespace Nodens
{

void EventBus::Unsubscribe(SubscriptionID id)
{
    std::lock_guard lock(m_RegistryMutex);

    for (auto& subscribers : m_Subscribers)
    {
        auto it = std::ranges::find(subscribers, id, &SubscriberEntry::Id);
        if (it != subscribers.end())
        {
            subscribers.erase(it);
            CoreLogger().debug("EventBus: Unsubscribed id={}.", id);
            return;
        }
    }

    CoreLogger().warn("EventBus: Attempted to unsubscribe unknown id={}.", id);
}

void EventBus::DispatchToSubscribers(EventTypeID typeId, void* eventData)
{
    ZoneScoped;

    // Copy the handler list so handlers can safely call Subscribe/Unsubscribe.
    std::vector<SubscriberEntry> subscribers;
    {
        std::lock_guard lock(m_RegistryMutex);
        if (typeId < m_Subscribers.size())
        {
            subscribers = m_Subscribers[typeId];
        }
    }

    CoreLogger().trace(
        "EventBus: Dispatching typeId={} to {} subscriber(s).", typeId, subscribers.size());
    for (auto& entry : subscribers)
    {
        entry.Handler(eventData);
    }
}

void EventBus::Flush()
{
    ZoneScoped;

    // Swap buffers: the current active buffer becomes the read buffer.
    std::size_t readIndex;
    std::size_t numEventTypes;
    {
        std::lock_guard lock(m_BufferMutex);
        readIndex = m_ActiveBufferIndex;
        m_ActiveBufferIndex = 1 - m_ActiveBufferIndex; // Swap buffer index (0 <-> 1)
        numEventTypes = m_Buffers[readIndex].size();
    }

    for (EventTypeID typeId = 0; typeId < numEventTypes; ++typeId)
    {
        auto& readBuffer = m_Buffers[readIndex][typeId];
        if (readBuffer.empty())
            continue;

        DispatchFn dispatchFn;
        {
            std::lock_guard lock(m_RegistryMutex);
            dispatchFn = m_DispatchTable[typeId];
            if (!dispatchFn)
            {
                CoreLogger().error("Dispatch function for Event type `{}` (typeId={}) not defined.",
                                   m_RegisteredEventsNames[typeId],
                                   typeId);
                continue;
            }
        }

        CoreLogger().trace("EventBus: Flushing {} bytes for `{}` (typeId={}).",
                           readBuffer.size(),
                           m_RegisteredEventsNames[typeId],
                           typeId);
        dispatchFn(readBuffer.data(), readBuffer.size());
        readBuffer.clear();
    }
}

} // namespace Nodens
