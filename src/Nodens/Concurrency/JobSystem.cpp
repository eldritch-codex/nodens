/// @file JobSystem.cpp
/// @brief Implementation of the JobSystem: thread spawning, shutdown, and worker loop.
/// @ingroup Concurrency

module;

#include <tracy/Tracy.hpp>

module Nodens.JobSystem;

import Nodens.Log;
import std;

namespace Nodens
{

JobSystem::JobSystem()
{
    // Calculate how many threads to spawn. We generally want to match the CPU
    // cores but leave one core free for the main thread to prevent hitching.
    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount > 1)
        threadCount--;

    m_Threads.reserve(threadCount);

    // Launch the workers.
    for (unsigned int i = 0; i < threadCount; ++i)
    {
        // std::jthread automatically passes a stop_token as the first argument
        // to the thread function if the signature accepts it.
        m_Threads.emplace_back(
            [this, i](std::stop_token stoken)
            {
                std::string name = "Worker " + std::to_string(i);
                tracy::SetThreadName(name.c_str());

                this->WorkerLoop(stoken);
            });
    }

    CoreLogger().info("JobSystem initialized with {} worker threads.", threadCount);
}

JobSystem::~JobSystem()
{
    // SHUTDOWN SEQUENCE
    // Ask all threads to stop. Ideally, they finish their current task and then
    // see this.
    for (auto& thread : m_Threads)
    {
        thread.request_stop();
    }

    // Wake up everyone, because if a thread is sleeping in 'm_Condition.wait', it
    // won't see the stop request until it wakes up. notify_all() forces them to
    // wake, check the token, and exit.
    m_Condition.notify_all();

    // Because they are std::jthread, they will automatically perform a .join()
    // waiting for the threads to actually finish execution before this object
    // dies.
}

void JobSystem::WorkerLoop(std::stop_token stoken)
{
    // The main loop for each worker thread. It continues as long as no stop is requested.
    while (!stoken.stop_requested())
    {
        // This will hold the task to be executed.
        std::move_only_function<void()> task;
        {
            // Lock the queue to safely access it.
            std::unique_lock lock(m_QueueMutex);

            // Wait on the condition variable. The thread will sleep until a new task is added
            // or a stop is requested. The lambda is a predicate that checks if the queue is not
            // empty. The wait will only proceed if the predicate is true or if a stop is requested.
            bool tasksAvailable =
                m_Condition.wait(lock, stoken, [this] { return !m_Tasks.empty(); });

            // If wait returns because a stop was requested and there are no tasks, exit the loop.
            if (!tasksAvailable && stoken.stop_requested())
            {
                return;
            }

            // Although the predicate should protect against this, a double check ensures thread
            // safety in case of spurious wakeups.
            if (m_Tasks.empty())
            {
                continue;
            }

            // A task is available, so move it from the queue to the local 'task' variable.
            task = std::move(m_Tasks.front());
            m_Tasks.pop();

            // Visualize queue size decreasing
            TracyPlot("Job Queue Size", (int64_t)m_Tasks.size());

            // Mark that this thread is currently holding the lock (Optional, for high contention
            // debug)
            LockMark(m_QueueMutex);
        }

        // Execute the task outside the lock to avoid holding the lock unnecessarily
        // and to allow other threads to queue up tasks.
        {
            ZoneScoped;
            if (task)
                task();
        }
    }
}

} // namespace Nodens
