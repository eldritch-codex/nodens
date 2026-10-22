/// @file JobSystem.cppm
/// @brief Multithreaded job system built on C++20 jthreads with future-based task submission.
/// @ingroup Concurrency

module;

#include <tracy/Tracy.hpp>

export module Nodens.JobSystem;

import std;

export namespace Nodens
{

/// @brief A multithreaded job system utilizing C++20 jthreads for automatic joining and
/// cancellation.
/// @details This system manages a pool of worker threads that consume tasks from a shared queue.
/// It uses a 'future-based' approach for task submission.
class JobSystem
{
public:
    /// @brief Initializes the JobSystem and launches worker threads.
    /// @details The number of threads spawned is equal to hardware_concurrency - 1
    /// to leave the main thread free for the OS/Application loop.
    JobSystem();

    /// @brief Destructor.
    /// @details Signals all threads to stop, wakes them up, and waits for them to join.
    /// Due to std::jthread, the joining happens automatically, but we explicitly signal stop first.
    ~JobSystem();

    /// @brief Submits a function (job) to the execution queue.
    /// @tparam F The type of the function object (must be invocable with Args...).
    /// @tparam Args The types of the arguments to pass to the function.
    /// @param f The function to execute.
    /// @param args The arguments to forward to the function.
    /// @return A std::future containing the result of the function execution.
    template <typename F, typename... Args>
        requires std::invocable<F, Args...>
    auto Submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock lock(m_QueueMutex);
            m_Tasks.emplace([task]() { (*task)(); });
            TracyPlot("Job Queue Size", (int64_t)m_Tasks.size());
        }

        m_Condition.notify_one();
        return res;
    }

private:
    /// @brief The main loop executed by every worker thread.
    /// @param stoken The C++20 stop_token used to check if a stop has been requested.
    void WorkerLoop(std::stop_token stoken);

private:
    /// @brief The thread-safe queue of tasks pending execution.
    std::queue<std::move_only_function<void()>> m_Tasks;

    /// @brief Mutex to protect access to m_Tasks.
    TracyLockable(std::mutex, m_QueueMutex);

    /// @brief Condition variable to put threads to sleep when there is no work.
    /// @note std::condition_variable_any is required to work with std::stop_token.
    std::condition_variable_any m_Condition;

    /// @brief Pool of worker threads.
    /// @note std::jthread (C++20) automatically joins on destruction.
    /// @warning Thread objects need to be declared after the resources they use to ensure
    /// proper destruction order.
    std::vector<std::jthread> m_Threads;
};

} // namespace Nodens
