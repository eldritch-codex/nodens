# Profiling with Tracy

Find out where a frame spends its time, using [Tracy Profiler](https://github.com/wolfpld/tracy).

> [!NOTE]
> **Mental model.** Tracy has two halves. A small *client* is compiled into your program and records timestamps. A separate *profiler* GUI connects to it over the network and draws the timeline. Nodens turns the client on only in the `Profiling` build configuration, so you profile a `Profiling` build, not a `Debug` or `Release` one.
>
> The page follows a profiling session:
>
> 1. **Build with the client.** [Build](#build-a-profiling-configuration)
> 2. **Connect the profiler.** [Connect](#connect-the-profiler)
> 3. **See what is already measured.** [Built-in instrumentation](#what-nodens-measures-for-you)
> 4. **Measure your own code.** [Zones](#your-own-zones) and [plots](#your-own-plots)
> 5. **Read the result.** [Reading a capture](#reading-a-capture)

Tracy gives you a frame timeline with spikes, nested timing per thread, lock contention, plots of any number over time, and Vulkan GPU timing (Nodens records the ImGui pass).

## Build a Profiling configuration

`Debug` and `Release` leave every Tracy macro as a no-op, so they produce no profiling data. `Profiling` mirrors Release code generation and adds debug symbols (`-g`) and frame pointers (`-fno-omit-frame-pointer`), so timings carry over to the shipped build and callstacks resolve.

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Profiling
cmake --build build
```

To build the profiler GUI from the fetched Tracy source, the `tracy-profiler` target is available unless `-DND_ADD_TRACY_PROFILER_TARGET=OFF` was passed. It is excluded from the default build, so build it by name:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Profiling
cmake --build build --target tracy-profiler
```

The profiler is a separate CMake project with its own dependencies, so it is excluded from the default build and is built inside the build tree, under the fetched Tracy sources. The development image includes what it needs (`capstone` and LLVM's archiving tools).

The client and the profiler must be the same Tracy version. The pinned version is in [`cmake/NodensDependencies.cmake`](../../cmake/NodensDependencies.cmake), currently `v0.14.1`; if you use a prebuilt profiler, take the matching release.

## Connect the profiler

1. Start the Tracy profiler GUI (before or after the application).
2. Run your `Profiling` build. The client listens on port **8086** from program start.
3. In the profiler, select the application from its client list, or enter the address and port.

You should see a frame timeline that advances once per frame, a track for the main thread, one track per task worker (`Worker 0`, `Worker 1`, ...) and, with Vulkan, GPU zones. In a `Debug` or `Release` build nothing listens, so there is nothing to connect to.

## What Nodens measures for you

- **Frame marks:** `FrameMark` at the end of `Application::Run()`.
- **CPU zones:** the application's construction, teardown and main loop; `EventBus` dispatch and flush; the `ImGuiLayer` frame; and every `TaskSystem` task.
- **Plot and lock:** "Job Queue Size", and the task queue mutex for contention views.
- **Vulkan GPU timing:** the ImGui pass. Validation layers are `Debug`-only, so they never slow a `Profiling` capture.

## Your own zones

Include Tracy's header in the global module fragment, then put a zone macro at the top of a scope:

```cpp
module;

#include <imgui.h>
#include <tracy/Tracy.hpp>

module MyApp.MyLayer;

import nodens;

void MyLayer::OnUpdate(Nodens::TimeStep ts)
{
    ZoneScoped; // Times this whole function, named after it

    {
        ZoneScopedN("Physics Update"); // A named zone for just this block
        // physics code...
    }

    {
        ZoneScopedNC("AI", 0x3399ff); // Named, with a color
        // AI code...
    }
}
```

`ZoneScoped` takes the function's name, `ZoneScopedN("name")` a custom one, and `ZoneScopedNC("name", color)` adds a `0xRRGGBB` color; each times until the end of its scope. Zones nest and appear on the thread that ran them, so a zone inside a [Concurrency and TaskSystem](./concurrency-and-tasksystem.md) task shows on that worker's track. Nodens already marks each frame, so you do not add `FrameMark`. In `Debug` and `Release` the macros expand to nothing, so leaving them in costs nothing.

## Your own plots

`TracyPlot` records a number against time:

```cpp
TracyPlot("Active Entities", static_cast<int64_t>(entityCount));
TracyPlot("Frame Budget Used", usedMilliseconds / 16.6f);
```

Call it whenever the value changes, typically once per frame. The name identifies the series, so reuse the same string.

## Reading a capture

Tracy's own documentation covers the interface. For a Nodens capture, start here:

| Look at | What it tells you |
|---|---|
| **Frame timeline** (top) | One bar per frame. A tall bar is a slow frame; click it to zoom in. |
| **Main thread track** | The nested `Application::Run` zones: event flush, layer updates, the ImGui frame, polling and presenting. The widest child of a slow frame is the bottleneck. |
| **`Worker N` tracks** | Which tasks ran where and for how long. Gaps mean idle workers. |
| **Job Queue Size plot** | A growing line means tasks are submitted faster than workers finish them. |
| **Lock information** | Contention on the task queue mutex. |
| **GPU zones** | How long the GPU spent on the ImGui pass, beside the CPU frame. |

## See Also

- [Concurrency and TaskSystem](./concurrency-and-tasksystem.md): the thread pool you are profiling.
- [Examples Guide](./examples-guide.md): `planetaryscan` submits many tasks, which makes the worker tracks and queue plot easy to see.
- [Tracy documentation](https://github.com/wolfpld/tracy)
