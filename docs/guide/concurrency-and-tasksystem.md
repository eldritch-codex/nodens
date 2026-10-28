# Concurrency and TaskSystem

How to do slow work without freezing the window: submit it as a task, then collect the result on the main thread.

> [!NOTE]
> **Mental model.** Picture a fixed crew of workers beside a task board. `Submit` pins your function to the board and hands you a claim slip, a `std::future`. A free worker takes the task, does it and fills in the slip. The main thread never stands around waiting: it glances at the slip once a frame, or receives a queued event when the work is done.
>
> This page follows a task from start to finish:
>
> 1. **Submit it.** [Submitting a task](#submitting-a-task)
> 2. **Collect the result.** [By polling a future](#collecting-by-polling-a-future) or [by event](#collecting-by-event)
> 3. **Stay safe.** [Thread-safety rules](#thread-safety-rules)
> 4. **Know the limits.** [What the pool does and does not do](#what-the-pool-does-and-does-not-do)

## Submitting a task

The `TaskSystem` is created and owned by the `Application`; get it with `Application::Get().GetTaskSystem()`. `Submit` takes any callable plus its arguments and returns a `std::future` of the result type:

```cpp
auto& tasks = Nodens::Application::Get().GetTaskSystem();

std::future<int> answer = tasks.Submit([] { return 6 * 7; });

// With arguments:
std::future<float> root = tasks.Submit(SlowSquareRoot, 2.0f);
```

- Arguments are captured **by value** at submit time, so a task never reads a caller's variable that has gone out of scope: an lvalue is copied, an rvalue is moved. The callable is moved in the same way, so a lambda holding a `std::unique_ptr` is accepted.
- A task that returns `void` gives a `std::future<void>`, still useful for "has it finished" checks and for carrying an exception.
- Dropping the future is fine: unlike the future from `std::async`, it does not block when destroyed, and the task still runs.

### Why not `std::bind`

`Submit` builds the packaged task itself, from a lambda with an init capture and `std::invoke`, instead of handing the callable and its arguments to `std::bind`. Three consequences follow, and they are the reason for the choice.

- **Value category survives.** `std::invoke` receives the callable and its arguments as rvalues, so an rvalue reaches a by-value parameter even when its type is move-only. `std::bind` stores a decayed copy of every argument and passes that copy back to the callable as an lvalue, so the same call needs a copy constructor.
- **One copy, at submit time.** An lvalue argument is copied once, straight into the task. Through `std::bind` it would be copied into the bind object and then again on the call.
- **One allocation fewer.** The queue holds `std::move_only_function<void()>`, and the packaged task moves straight into it. The packaged task still keeps its own shared state for the result, but the separate `std::shared_ptr<std::packaged_task>` and its control block are gone.

## Collecting by polling a future

Keep the future as a member, check it without blocking, and take the result when it is ready:

```cpp
float SlowSquareRoot(float x)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return std::sqrt(x);
}

class TaskDemoLayer : public Nodens::ILayer
{
public:
    TaskDemoLayer() : ILayer("TaskDemo") {}

    void OnUpdate(Nodens::TimeStep) override
    {
        if (m_Result.valid() &&
            m_Result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            m_Value = m_Result.get(); // Takes the result and invalidates the future.
        }
    }

    void OnImGuiRender(Nodens::TimeStep) override
    {
        ImGui::Begin("Tasks");
        if (ImGui::Button("Compute") && !m_Result.valid())
            m_Result = Nodens::Application::Get().GetTaskSystem().Submit(SlowSquareRoot, 2.0f);

        if (m_Result.valid())
            ImGui::TextUnformatted("Working...");
        else
            ImGui::Text("Last result: %.4f", m_Value);
        ImGui::End();
    }

private:
    std::future<float> m_Result;
    float m_Value{0.0f};
};
```

Why it is written this way:

- **`wait_for(0)`, not `get()`.** `get()` blocks until the task finishes, which would freeze the window. `wait_for` with a zero timeout only asks "is it done?".
- **`valid()` first.** A default-constructed future, or one already `get()`-ed, is invalid, and calling `wait_for` on it is undefined. The same check stops a second click from submitting a second task.
- **Exceptions travel through the future.** If the task throws, `get()` rethrows on the main thread, so wrap it in `try` if the task can fail.

## Collecting by event

A future suits one task and one consumer. When several tasks report to a layer that is already listening, queue an event from the worker instead. The PlanetaryScan example does this: the layer subscribes in `OnAttach`, each task ends with `EventBus::QueueEvent`, and the bus delivers the results on the main thread at the start of the next frame.

```cpp
Nodens::Application::Get().GetTaskSystem().Submit(
    [id]()
    {
        PlanetaryScanResultEvent e{.m_ID = id};
        // ... heavy work, then fill in e ...
        Nodens::Application::Get().GetEventBus().QueueEvent<PlanetaryScanResultEvent>(e);
    });
```

See [Event System](./event-system.md) for events and [Examples Guide](./examples-guide.md) for the whole example.

## Thread-safety rules

| Safe from a worker thread | Not safe from a worker thread |
|---|---|
| Pure computation on your own data | Window calls and anything touching the native window |
| `TaskSystem::Submit` | Rendering and graphics backend calls |
| `EventBus::QueueEvent` | Any `ImGui::` or `ImPlot::` call |
| Reading `Application::Get()` | Pushing layers or touching the `LayerStack` |
| Writing to your own data behind a mutex | Calling `EventBus::Flush` yourself (the application does it once per frame) |

Two more rules:

- **Protect shared data.** A layer's members are read on the main thread and may be written by a task. Guard them with a `std::mutex`, as the PlanetaryScan layer does, or hand data over through a future or a queued event.
- **`EventBus::Dispatch` runs on the calling thread.** Called from a worker, subscribers run on that worker. Use `QueueEvent` when the handler must run on the main thread.

## What the pool does and does not do

- **Size.** `hardware_concurrency() - 1` workers, so one core stays free for the main thread (one worker if the machine reports a single hardware thread). They are named `Worker 0`, `Worker 1`, and so on, which shows as separate tracks in Tracy.
- **Why a pool.** Starting a thread costs far more than running a short task, and unbounded threads would fight the main thread for cores.
- **Shutdown.** When the `Application` is destroyed, each worker finishes the task it is running and exits. Do not rely on a task that is still queued: it may start just as shutdown begins, or it is discarded, and `get()` on the future of a discarded task throws a `std::future` error with the condition `broken_promise`.
- **Not provided in 0.1.1.** There are no task priorities, no cancelling a queued task and no dependencies between tasks. Build those on top of futures if you need them.
- **Profiling.** With the `Profiling` build, the queue length is plotted as "Job Queue Size", the queue mutex shows contention, and each task is a zone. Add your own with `ZoneScopedN("My Task")`. See [Profiling with Tracy](./profiling-with-tracy.md).

## See Also

- [Architecture Overview](./architecture-overview.md): where the `TaskSystem` fits in the threading model.
- [Event System](./event-system.md): queued events, the other way back to the main thread.
- [Profiling with Tracy](./profiling-with-tracy.md): visualizing worker activity.
