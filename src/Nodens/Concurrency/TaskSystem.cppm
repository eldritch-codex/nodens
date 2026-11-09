/// @file TaskSystem.cppm
/// @brief Multithreaded task system built on C++20 jthreads with future-based task submission.
/// @ingroup Concurrency

module;

#include <tracy/Tracy.hpp>

export module Nodens.TaskSystem;

import std;

export namespace Nodens
{

/// @brief What SubmitStoppable returns: the result and the means to stop the task.
/// @tparam TResult The type the task returns.
template <typename TResult>
struct FStoppableTaskHandle
{
    /// @brief Receives the task's result, or the exception it threw.
    std::future<TResult> Result;

    /// @brief Request a stop on this source to ask the task to finish early.
    std::stop_source Stop;
};

/// @brief A multithreaded task system utilizing C++20 jthreads for automatic joining and
/// cancellation.
/// @details This system manages a pool of worker threads that consume tasks from a shared queue.
/// It uses a 'future-based' approach for task submission.
class TaskSystem
{
public:
    /// @brief Initializes the TaskSystem and launches worker threads.
    /// @details The number of threads spawned is equal to hardware_concurrency - 1
    /// to leave the main thread free for the OS/Application loop.
    TaskSystem();

    /// @brief Destructor.
    /// @details Signals all threads to stop, wakes them up, and waits for them to join.
    /// Due to std::jthread, the joining happens automatically, but we explicitly signal stop first.
    ~TaskSystem();

    /// @brief Submits a function (task) to the execution queue.
    /// @tparam F The type of the function object (must be invocable with Args...).
    /// @tparam Args The types of the arguments to pass to the function.
    /// @param f The function to execute.
    /// @param args The arguments to forward to the function.
    /// @return A std::future containing the result of the function execution.
    /// @note The callable and its arguments move into the task, so both may be move-only. A
    ///       std::reference_wrapper still refers to the caller's object, which must outlive it.
    template <typename F, typename... Args>
        requires std::invocable<std::decay_t<F>, std::decay_t<Args>...>
    auto Submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    {
        using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        std::packaged_task<return_type()> task{
            [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable
            { return std::invoke(std::move(f), std::move(args)...); }};

        auto res = task.get_future();
        Enqueue(std::move(task));
        return res;
    }

    /// @brief Submits a task that receives a std::stop_token as its first argument.
    /// @details The token is stopped when the caller requests a stop on the returned handle, and
    /// when the TaskSystem is destroyed while the task is running. A task that never checks its
    /// token still delays the destructor until it returns.
    /// @tparam F The type of the function object (must be invocable with a std::stop_token, then
    /// Args...).
    /// @tparam Args The types of the arguments to pass after the token.
    /// @param f The function to execute.
    /// @param args The arguments to forward to the function, after the token.
    /// @return A handle holding the std::future of the result and the std::stop_source of the
    /// task.
    /// @note A task still queued when the TaskSystem is destroyed never runs, as with Submit.
    template <typename F, typename... Args>
        requires std::invocable<std::decay_t<F>, std::stop_token, std::decay_t<Args>...>
    auto SubmitStoppable(F&& f, Args&&... args) -> FStoppableTaskHandle<
        std::invoke_result_t<std::decay_t<F>, std::stop_token, std::decay_t<Args>...>>
    {
        using return_type =
            std::invoke_result_t<std::decay_t<F>, std::stop_token, std::decay_t<Args>...>;

        std::stop_source stop;

        std::packaged_task<return_type()> task{
            [f = std::forward<F>(f),
             ... args = std::forward<Args>(args),
             stop,
             shutdown = m_Shutdown.get_token()]() mutable
            {
                // A stop_token cannot be combined with another, so the shutdown request is
                // forwarded to this task's own source for as long as the task runs.
                std::stop_callback forward(shutdown, [stop]() mutable { stop.request_stop(); });
                return std::invoke(std::move(f), stop.get_token(), std::move(args)...);
            }};

        auto res = task.get_future();
        Enqueue(std::move(task));
        return {std::move(res), std::move(stop)};
    }

private:
    /// @brief Adds a task to the queue and wakes one worker.
    /// @param task The task to run on a worker.
    void Enqueue(std::move_only_function<void()> task);

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

    /// @brief Requested first when the TaskSystem is destroyed; running stoppable tasks see it.
    std::stop_source m_Shutdown;

    /// @brief Pool of worker threads.
    /// @note std::jthread (C++20) automatically joins on destruction.
    /// @warning Thread objects need to be declared after the resources they use to ensure
    /// proper destruction order.
    std::vector<std::jthread> m_Threads;
};

} // namespace Nodens
