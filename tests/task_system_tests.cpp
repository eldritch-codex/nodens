/// @file task_system_tests.cpp
/// @brief Tests for TaskSystem.

#include <gtest/gtest.h>

import nodens;
import std;

using namespace std::chrono_literals;

namespace
{
/// Waits for a stop request, the way a long task polls its token.
bool WaitForStop(std::stop_token token)
{
    while (!token.stop_requested())
        std::this_thread::sleep_for(1ms);
    return true;
}
} // namespace

TEST(TaskSystem, SubmitReturnsTheResult)
{
    Nodens::TaskSystem tasks;

    auto answer = tasks.Submit([] { return 6 * 7; });
    EXPECT_EQ(answer.get(), 42);

    auto square = tasks.Submit([](int x) { return x * x; }, 9);
    EXPECT_EQ(square.get(), 81);
}

TEST(TaskSystem, SubmitRunsAVoidTask)
{
    Nodens::TaskSystem tasks;
    bool ran = false;

    tasks.Submit([&ran] { ran = true; }).get();

    EXPECT_TRUE(ran);
}

TEST(TaskSystem, SubmitAcceptsAMoveOnlyArgument)
{
    Nodens::TaskSystem tasks;

    auto result =
        tasks.Submit([](std::unique_ptr<int> p) { return *p + 1; }, std::make_unique<int>(5));

    EXPECT_EQ(result.get(), 6);
}

TEST(TaskSystem, SubmitCarriesAnExceptionThroughTheFuture)
{
    Nodens::TaskSystem tasks;

    auto failing = tasks.Submit([]() -> int { throw std::runtime_error("boom"); });

    EXPECT_THROW(failing.get(), std::runtime_error);
}

TEST(TaskSystemStoppable, PassesTheTokenThenTheArguments)
{
    Nodens::TaskSystem tasks;

    auto handle = tasks.SubmitStoppable([](std::stop_token token, int a, std::unique_ptr<int> b)
                                        { return !token.stop_requested() ? a + *b : -1; },
                                        2,
                                        std::make_unique<int>(3));

    EXPECT_EQ(handle.Result.get(), 5);
}

TEST(TaskSystemStoppable, TheHandleStopsTheTask)
{
    Nodens::TaskSystem tasks;

    auto handle = tasks.SubmitStoppable(WaitForStop);
    EXPECT_NE(handle.Result.wait_for(50ms), std::future_status::ready);

    handle.Stop.request_stop();

    ASSERT_EQ(handle.Result.wait_for(5s), std::future_status::ready);
    EXPECT_TRUE(handle.Result.get());
}

TEST(TaskSystemStoppable, ShutdownStopsARunningTask)
{
    std::promise<void> started;
    std::future<bool> result;
    const auto begin = std::chrono::steady_clock::now();
    {
        Nodens::TaskSystem tasks;
        auto handle = tasks.SubmitStoppable(
            [&started](std::stop_token token)
            {
                started.set_value();
                return WaitForStop(token);
            });
        ASSERT_EQ(started.get_future().wait_for(5s), std::future_status::ready);
        result = std::move(handle.Result);
    } // the destructor must stop the task, or it would wait here for ever

    EXPECT_LT(std::chrono::steady_clock::now() - begin, 5s);
    ASSERT_EQ(result.wait_for(0s), std::future_status::ready);
    EXPECT_TRUE(result.get());
}

TEST(TaskSystemStoppable, CarriesAnExceptionThroughTheFuture)
{
    Nodens::TaskSystem tasks;

    auto handle =
        tasks.SubmitStoppable([](std::stop_token) -> int { throw std::runtime_error("boom"); });

    EXPECT_THROW(handle.Result.get(), std::runtime_error);
}

TEST(TaskSystemStoppable, SubmitIsUnchanged)
{
    Nodens::TaskSystem tasks;

    EXPECT_EQ(tasks.Submit([](int x) { return x + 1; }, 1).get(), 2);
}
