# Profiling with Tracy

Find out where an iteration spends its time, using [Tracy Profiler](https://github.com/wolfpld/tracy).

> [!NOTE]
> **Mental model.** Tracy has two halves. A small *client* is compiled into your program and records timestamps. A separate *profiler* GUI connects to it over the network and draws the timeline. Nodens turns the client on only in the `Profiling` build configuration, so you profile a `Profiling` build, never a `Debug` or `Release` one.

Tracy shows a timeline with spikes, nested timing per thread, lock contention, plots of any number over time, and Vulkan GPU timing for the ImGui pass.

## Prerequisites

| You need | Why |
|---|---|
| A `Profiling` build of your application | `Debug` and `Release` compile every Tracy macro to nothing |
| The profiler GUI at the same Tracy version | The client and the profiler must match; the pin is in `cmake/NodensDependencies.cmake`, currently `v0.14.1` |
| Port 8086 free between the two | The client listens there from program start |

## Steps

### 1. Configure a Profiling build

`Profiling` mirrors Release code generation and adds debug symbols and frame pointers, so timings carry over to the shipped build and callstacks resolve.

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Profiling
cmake --build build
```

**Success looks like** a build that completes, with Tracy compiled in.

### 2. Build the profiler GUI, if you do not have one

The fetched Tracy source provides the GUI as its own target, excluded from the default build:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Profiling
cmake --build build --target tracy-profiler
```

**Success looks like** an executable under the build directory, inside the fetched Tracy sources. `-DND_ADD_TRACY_PROFILER_TARGET=OFF` removes the target; the development image carries what the GUI needs, `capstone` and LLVM's archiving tools.

### 3. Start the profiler GUI

Run it before or after the application; the order does not matter. A prebuilt profiler works too, as long as its version matches the pin.

### 4. Run your application

```shell
./build/NodensApp
```

**Success looks like** the application appearing in the profiler's client list, which you then select, or an address and port 8086 you can enter by hand. The timeline advances once per iteration, with a track for the main thread, one per task worker and, with Vulkan, GPU zones.

### 5. Measure your own code

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

**Success looks like** the zone appearing on the track of the thread that ran it, which for a task is that worker's track. `ZoneScoped` takes the function's name, `ZoneScopedN` a custom one, and `ZoneScopedNC` adds a `0xRRGGBB` color; each ends with its scope. Nodens already marks each iteration, so you do not add `FrameMark`.

### 6. Plot a number

```cpp
TracyPlot("Active Entities", static_cast<int64_t>(entityCount));
TracyPlot("Iteration Budget Used", usedMilliseconds / 16.6f);
```

Call it when the value changes, typically once per iteration, and reuse the same name string: the name is the identity of the series. **Success looks like** a new line in the profiler's plot list.

### 7. Read the capture

| Look at | What it tells you |
|---|---|
| **Timeline** | One bar per iteration. A tall bar is a slow one; click it to zoom in. |
| **Main thread track** | The nested `Application::Run` zones: event flush, layer updates, the ImGui frame, polling and presenting. The widest child of a slow iteration is the bottleneck. |
| **`Worker N` tracks** | Which tasks ran where and for how long. Gaps mean idle workers. |
| **Job Queue Size plot** | A growing line means tasks arrive faster than workers finish them. |
| **Lock information** | Contention on the task queue mutex. |
| **GPU zones** | How long the GPU spent on the ImGui pass, beside the CPU side. |

## What Nodens records already

- **Iteration marks:** `FrameMark` at the end of each turn of `Application::Run()`.
- **CPU zones:** construction, teardown and the main loop; `EventBus` dispatch and flush; the `ImGuiLayer` frame; every `TaskSystem` task.
- **Plot and lock:** "Job Queue Size", and the task queue mutex for the contention views.
- **Vulkan GPU timing:** the ImGui pass. Validation layers exist only in `Debug`, so they never slow a `Profiling` capture.

## Troubleshooting

**The application never appears in the client list.** Check the build type first: in `Debug` and `Release` nothing listens, so there is nothing to connect to.

**The profiler refuses to connect or shows broken data.** The client and the GUI are different Tracy versions. Use the pin in `cmake/NodensDependencies.cmake`, or the prebuilt release that matches it.

**No GPU zones.** The build has no Vulkan backend, so the ImGui pass is not recorded as GPU work. [Building and Toolchain](./building-and-toolchain.md) covers the backend check.

**Empty or missing callstacks.** The build is not `Profiling`, which is the only configuration that keeps debug symbols and frame pointers.

## See Also

- [Concurrency and TaskSystem](./concurrency-and-tasksystem.md): the thread pool whose worker tracks you are reading.
- [Examples Guide](./examples-guide.md): `planetaryscan` submits many tasks, which makes the worker tracks and queue plot easy to see.
- [Tracy documentation](https://github.com/wolfpld/tracy)
