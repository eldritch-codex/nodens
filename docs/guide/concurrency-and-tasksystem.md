# Concurrency and TaskSystem

How to do slow work without freezing the window: submit it as a task, then collect the result on the main thread.

> [!NOTE]
> **Mental model.** Picture a fixed crew of workers beside a task board. `Submit` pins your function to the board and hands you a claim slip, a `std::future`. A free worker takes the task, does it and fills in the slip. The main thread never stands around waiting: it glances at the slip once per iteration, or receives a queued event when the work is done.

## Problem

The main thread runs everything the user can see: window messages, rendering, ImGui and every layer callback. A slow computation placed there stops all of it, and a window that stops replying looks broken.

The obvious answer, a thread per task, fails for its own reasons. Starting a thread costs far more than running a short task, and a hundred threads compete with the main thread for the same cores, which slows the work that must stay fast.

The `TaskSystem` is one pool of workers, created and owned by the `Application`, with a result handed back in a form the main thread can collect without blocking.

## How it works

### Submitting a task

Get the pool with `Application::Get().GetTaskSystem()`. `Submit` takes any callable plus its arguments and returns a `std::future` of the result type:

```cpp
auto& tasks = Nodens::Application::Get().GetTaskSystem();

std::future<int> answer = tasks.Submit([] { return 6 * 7; });

// With arguments:
std::future<float> root = tasks.Submit(SlowSquareRoot, 2.0f);
```

A task is one callable plus its arguments, captured by value at submit time. The callable is moved in, so a lambda holding a `std::unique_ptr` is accepted.

- Arguments are captured **by value**, so a task never reads a caller's variable that has gone out of scope: an lvalue is copied, an rvalue is moved.
- A task that returns `void` gives a `std::future<void>`, still useful for a finished check and for carrying an exception.
- Dropping the future is fine: unlike the future from `std::async`, it does not block when destroyed, and the task still runs.

### The pool

- **Size.** `hardware_concurrency() - 1` workers, so one core stays free for the main thread, and one worker if the machine reports a single hardware thread.
- **Names.** `Worker 0`, `Worker 1`, and so on, which appear as separate tracks in Tracy.
- **Shutdown.** When the `Application` is destroyed, each worker finishes the task it is running and exits.

### Why `Submit` does not use `std::bind`

`Submit` builds the packaged task itself, from a lambda with an init capture and `std::invoke`, instead of handing the callable and its arguments to `std::bind`. Three consequences follow, and they are the reason for the choice.

- **Value category survives.** `std::invoke` receives the callable and its arguments as rvalues, so an rvalue reaches a by-value parameter even when its type is move-only. `std::bind` stores a decayed copy of every argument and passes that copy back to the callable as an lvalue, so the same call needs a copy constructor.
- **One copy, at submit time.** An lvalue argument is copied once, straight into the task. Through `std::bind` it would be copied into the bind object and then again on the call.
- **One allocation fewer.** The queue holds `std::move_only_function<void()>`, and the packaged task moves straight into it. The packaged task still keeps its own shared state for the result, but the separate `std::shared_ptr<std::packaged_task>` and its control block are gone.

### Collecting by polling a future

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

### Collecting by event

A future suits one task and one consumer. When several tasks report to a layer that is already listening, queue an event from the worker instead:

```cpp
Nodens::Application::Get().GetTaskSystem().Submit(
    [id]()
    {
        PlanetaryScanResultEvent e{.m_ID = id};
        // ... heavy work, then fill in e ...
        Nodens::Application::Get().GetEventBus().QueueEvent<PlanetaryScanResultEvent>(e);
    });
```

The PlanetaryScan example does this: the layer subscribes in `OnAttach`, each task ends with `QueueEvent`, and the bus delivers the results on the main thread at the start of the next iteration. [Event System](./event-system.md) covers the bus, and [Examples Guide](./examples-guide.md) the whole example.

## What still goes wrong

**A blocking `get()` on the main thread.** It waits for the task, which freezes the window for exactly as long as the work takes.

**`wait_for` on an invalid future.** A default-constructed future, or one already collected, is invalid, and calling `wait_for` on it is undefined.

**A worker touching the UI.** Window calls, rendering, and every `ImGui::` and `ImPlot::` call belong to the main thread.

**Shared members read and written from both sides.** A layer's members are read on the main thread while a task may write them.

**A queued task assumed to run at shutdown.** A task that has not started may be discarded when the application ends, and `get()` on its future throws with the condition `broken_promise`.

**An exception lost in the future.** A task that throws rethrows on the main thread when the result is collected, and nowhere else.

**Priorities, cancellation or dependencies.** The pool provides none of them.

## Remedies

**Poll with `wait_for(0)` and collect when ready.** That is the "is it done" question, and it costs nothing when the answer is no.

**Check `valid()` first.** It guards the invalid case and stops a second submission while one is in flight.

**Keep UI work on the main thread.** A worker computes, then reports through a queued event or a future the main thread reads.

**Guard shared data with a mutex, or hand it over.** Copy the input into the task, and send the result back through the future or the bus, so no member is touched by two threads at once.

**Do not depend on an unstarted task at shutdown.** Wait for the results the application still needs before it ends.

**Collect every future you rely on.** An exception surfaces only when the result is read.

**Build missing behaviour on futures.** Priorities, cancellation and dependencies are application-level concerns on top of `Submit`, not pool features.

## A worked case

The `TaskDemoLayer` above, over four iterations, with a task that sleeps 500 ms:

| Iteration | What the layer does | What the user sees |
|---|---|---|
| 1 | The button is pressed and `Submit` returns a future | "Working..." |
| 2 | `wait_for(0)` reports `timeout` | "Working..." |
| 30 | `wait_for(0)` reports `ready`, `get()` takes 1.4142 | "Last result: 1.4142" |

The number in the third row is `sqrt(2)`, and the row count varies with the frame rate: at 60 Hz about thirty iterations pass in half a second, at 20 Hz about ten. Nothing in the layer depends on that count, because the check asks the future rather than counting iterations.

With the event form, the same work looks like this:

| Step | Thread | What happens |
|---|---|---|
| 1 | main | The layer subscribes in `OnAttach` |
| 2 | main | A button submits a task |
| 3 | worker | The task computes and calls `QueueEvent` |
| 4 | main | The next iteration flushes and the handler runs |

## Choosing

- **A future or an event?** Use a future for one task and one consumer that polls. Use the bus when several tasks report to one listener, or when the producer should not know the consumer.
- **Poll or block?** Poll, unless the result is needed before anything else can proceed, and even then block only off the main thread.
- **How small a task?** Big enough that the submit cost and the copy are a small share of the work. A task that takes microseconds is cheaper to run inline.

## Rules

1. Nothing that draws, polls a window or touches ImGui runs on a worker.
2. `Submit` and `QueueEvent` are safe from any thread; `Dispatch` and `Flush` are not.
3. Capture by value, and prefer moving data into the task over sharing it.
4. Poll futures with `wait_for(0)`, and collect every result you depend on.
5. Do not wait for an unstarted task at shutdown.

## See Also

- [Architecture Overview](./architecture-overview.md): where the `TaskSystem` fits in the threading model.
- [Event System](./event-system.md): queued events, the other way back to the main thread.
- [Profiling with Tracy](./profiling-with-tracy.md): queue length, contention and worker activity.
