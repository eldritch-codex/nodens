/// @file event_bus_tests.cpp
/// @brief Tests for EventBus.

#include <gtest/gtest.h>

import nodens;
import std;

namespace
{
// Every test that needs an event type defines its own. A type's ID is a process-wide counter that
// each type takes the first time it is used, so sharing a type between tests would make the
// tests depend on the order they run in.

struct DispatchedEvent
{
    static constexpr std::string_view Name = "DispatchedEvent";
    int Value{};
};

struct QueuedEvent
{
    static constexpr std::string_view Name = "QueuedEvent";
    int Value{};
};

struct UnsubscribedEvent
{
    static constexpr std::string_view Name = "UnsubscribedEvent";
    int Value{};
};

struct FirstOrderEvent
{
    static constexpr std::string_view Name = "FirstOrderEvent";
    int Value{};
};

struct SecondOrderEvent
{
    static constexpr std::string_view Name = "SecondOrderEvent";
    int Value{};
};

struct TwoBusEvent
{
    static constexpr std::string_view Name = "TwoBusEvent";
    int Value{};
};
} // namespace

TEST(EventBus, DispatchCallsTheSubscriberBeforeItReturns)
{
    Nodens::EventBus bus;
    int received = 0;
    const auto id =
        bus.Subscribe<DispatchedEvent>([&received](DispatchedEvent& e) { received = e.Value; });

    DispatchedEvent event{.Value = 7};
    bus.Dispatch(event);

    EXPECT_EQ(received, 7);
    bus.Unsubscribe(id);
}

TEST(EventBus, AQueuedEventWaitsForFlush)
{
    Nodens::EventBus bus;
    int received = 0;
    const auto id =
        bus.Subscribe<QueuedEvent>([&received](QueuedEvent& e) { received += e.Value; });

    bus.QueueEvent<QueuedEvent>({.Value = 3});
    EXPECT_EQ(received, 0);

    bus.Flush();
    EXPECT_EQ(received, 3);
    bus.Unsubscribe(id);
}

TEST(EventBus, UnsubscribeStopsDelivery)
{
    Nodens::EventBus bus;
    int received = 0;
    const auto id =
        bus.Subscribe<UnsubscribedEvent>([&received](UnsubscribedEvent&) { ++received; });

    bus.Unsubscribe(id);

    UnsubscribedEvent event{};
    bus.Dispatch(event);
    bus.QueueEvent<UnsubscribedEvent>({});
    bus.Flush();
    EXPECT_EQ(received, 0);
}

// A type's ID is assigned in the order the types are first used anywhere in the process, which is
// not the order a bus sees them. Subscribing the type with the higher ID first used to leave the
// other type without a dispatch function, so its queued events were never delivered.
TEST(EventBus, QueuedEventsAreDeliveredWhateverOrderTheTypesRegisterIn)
{
    const auto firstId = Nodens::GetEventTypeId<FirstOrderEvent>();
    const auto secondId = Nodens::GetEventTypeId<SecondOrderEvent>();
    ASSERT_NE(firstId, secondId);

    Nodens::EventBus bus;
    int first = 0;
    int second = 0;
    auto subscribeFirst = [&]
    { return bus.Subscribe<FirstOrderEvent>([&first](FirstOrderEvent& e) { first += e.Value; }); };
    auto subscribeSecond = [&]
    {
        return bus.Subscribe<SecondOrderEvent>([&second](SecondOrderEvent& e)
                                               { second += e.Value; });
    };

    // Register the type with the higher ID first.
    if (firstId > secondId)
    {
        (void)subscribeFirst();
        (void)subscribeSecond();
    }
    else
    {
        (void)subscribeSecond();
        (void)subscribeFirst();
    }

    bus.QueueEvent<FirstOrderEvent>({.Value = 1});
    bus.QueueEvent<SecondOrderEvent>({.Value = 10});
    bus.Flush();

    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 10);
}

// A type's ID belongs to the process, but a bus only knows the types it has registered. Each bus
// must register the types it uses, whatever other buses have done.
TEST(EventBus, EachBusRegistersTheTypesItUses)
{
    int onFirstBus = 0;
    int onSecondBus = 0;

    Nodens::EventBus first;
    (void)first.Subscribe<TwoBusEvent>([&onFirstBus](TwoBusEvent& e) { onFirstBus += e.Value; });
    first.QueueEvent<TwoBusEvent>({.Value = 1});
    first.Flush();

    Nodens::EventBus second;
    (void)second.Subscribe<TwoBusEvent>([&onSecondBus](TwoBusEvent& e) { onSecondBus += e.Value; });
    second.QueueEvent<TwoBusEvent>({.Value = 2});
    second.Flush();

    EXPECT_EQ(onFirstBus, 1);
    EXPECT_EQ(onSecondBus, 2);
}
