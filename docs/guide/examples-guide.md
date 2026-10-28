# Examples Guide

Two small programs in the Nodens repository, each the smallest complete application for one idea. Read them after the concept pages, or alongside them. The README has a [short overview](../../README.md#example-applications).

> [!NOTE]
> **Mental model.** Each example is an application class that pushes a single layer. Learn the shape once, then read what the layer does.
>
> | Example | The idea it shows | Read first |
> |---|---|---|
> | [`circularwave3d`](#circularwave3d) | **Drawing:** layer state plus ImPlot and ImPlot3D | [Using ImGui and ImPlot](./using-imgui-and-implot.md) |
> | [`planetaryscan`](#planetaryscan) | **Concurrency:** tasks on workers, results back through the event bus | [Concurrency and TaskSystem](./concurrency-and-tasksystem.md), [Event System](./event-system.md) |
>
> Then [how to run them](#running-the-examples).

Both examples have the same shape. They live in `examples/<name>/` in the Nodens repository and consist of:

- an application file with `main()` and a `Nodens::Application` subclass that pushes the layer, and
- a layer module (`.cppm`) plus its implementation (`.cpp`).

## circularwave3d

**Demonstrates:** an `ILayer` with per-frame state, immediate mode GUI, and 2D and 3D plotting with ImPlot and ImPlot3D.

It opens an 800 by 600 window with two panels: **ImPlot Example** draws the sine and cosine components of a circular wave as two lines, and **ImPlot3D Example** draws the same wave as one rotatable 3D curve with the three axes `t`, `x` and `y`.

### How it works

The layer owns three arrays of 1000 floats: time `t`, and the wave's `xs` and `ys`.

- **Constructor.** Fills the arrays: `xs = sin(f * t)` and `ys = cos(f * t)`, with a frequency `f` of 5π.
- **`OnUpdate`.** Advances every `t` by a small increment and recomputes `xs` and `ys`, so the curve scrolls.
- **`OnImGuiRender`.** Opens two ImGui windows, one with an `ImPlot::BeginPlot` and `PlotLine` for each component, one with an `ImPlot3D::BeginPlot` and a single `PlotLine` over `t`, `xs` and `ys`. Both plots use `AutoFit` axes so they follow the moving data.

The application file is the template every example follows:

```cpp
class CircularWave3DApp : public Nodens::Application
{
public:
    static inline const Nodens::FApplicationConfig appConfig = {
        .Name = "[NodensApp Example] CircularWave3D",
        .WindowWidth = 800,
        .WindowHeight = 600,
        .EnableGUI = true,
    };

    CircularWave3DApp() : Application(appConfig)
    {
        PushLayer(new CircularWave3DLayer());
    }
};
```

`OnUpdate` advances the curve by `speed * ts`, using the `TimeStep`, so it scrolls at the same speed at any frame rate ([Layer System](./layer-system.md)).

The plotting calls are shown and explained in [Using ImGui and ImPlot](./using-imgui-and-implot.md).

### Key Files

- [`examples/circularwave3d/CircularWave3DApp.cpp`](../../examples/circularwave3d/CircularWave3DApp.cpp): Application entry point
- [`examples/circularwave3d/CircularWave3DLayer.cppm`](../../examples/circularwave3d/CircularWave3DLayer.cppm): Layer interface
- [`examples/circularwave3d/CircularWave3DLayer.cpp`](../../examples/circularwave3d/CircularWave3DLayer.cpp): Layer implementation
- [`examples/circularwave3d/CMakeLists.txt`](../../examples/circularwave3d/CMakeLists.txt): How the module file is registered as a `CXX_MODULES` file set

---

## planetaryscan

**Demonstrates:** submitting tasks to the `TaskSystem`, reporting results back through the `EventBus`, a mutex-protected data model, and live scatter and bar plots.

It opens an 800 by 600 window with two panels. **Deep Space Analytics** has two buttons, **Launch Single Probe** and **Launch Swarm (20 Probes)**, and a scatter plot of each finished scan's distance against atmosphere density. **Performance Monitor** shows a bar chart of how long each recent task took.

### The event flow

```mermaid
sequenceDiagram
  participant UI as Main thread (button)
  participant JS as TaskSystem worker
  participant EB as EventBus
  participant L as PlanetaryScanLayer
  UI->>JS: Submit(scan task)
  Note over JS: sleeps 0.5 to 2.5 s,<br/>generates random data
  JS->>EB: QueueEvent(PlanetaryScanResultEvent)
  Note over EB: held until the next frame
  EB->>L: Flush() calls the subscriber on the main thread
  L->>L: AddResult() under the mutex
```

1. **The event.** `PlanetaryScanResultEvent` is a plain struct holding an ID, a distance, an atmosphere density and a calculation time, plus the `Name` string every event needs. A `std::formatter` specialization next to it makes the event printable in log calls. See [Event System](./event-system.md) for the rules events follow.
2. **Subscribing.** In `OnAttach` the layer calls `Application::Get().GetEventBus().Subscribe<PlanetaryScanResultEvent>(...)` with a lambda that calls `AddResult`.
3. **Launching a task.** A button press calls `LaunchScanJob`, which uses `Application::Get().GetTaskSystem().Submit(...)` to run a lambda on a worker. The task sleeps for a random 0.5 to 2.5 seconds to imitate a slow computation, generates random scientific data, measures its own duration, and fills in an event.
4. **Reporting back.** The task ends with `GetEventBus().QueueEvent<PlanetaryScanResultEvent>(e)`. `QueueEvent` is safe from a worker thread and only stores the event.
5. **Delivery.** At the start of the next frame, `Application::Run` calls `EventBus::Flush()`, which invokes the layer's subscriber **on the main thread**.
6. **Storing the result.** `AddResult` takes the layer's mutex, appends the distance, density and latency, and trims the latency history to the last 100 entries so memory stays bounded.
7. **Drawing.** `OnImGuiRender` locks the same mutex while it plots, so the lists cannot change mid-draw.

### Things to try

- Press **Launch Swarm (20 Probes)**. With fewer idle workers than probes, the surplus tasks wait in the queue, and results arrive in waves as workers free up. Run a `Profiling` build (see [Profiling with Tracy](./profiling-with-tracy.md)) and the **Job Queue Size** plot shows the backlog drain.
- Watch the **Performance Monitor** bars: each bar is one task's duration, which is the random sleep, so the bars vary between about 0.5 and 2.5 seconds.

### Key Files

- [`examples/planetaryscan/PlanetaryScanApp.cpp`](../../examples/planetaryscan/PlanetaryScanApp.cpp): Application entry point
- [`examples/planetaryscan/PlanetaryScanLayer.cppm`](../../examples/planetaryscan/PlanetaryScanLayer.cppm): The event struct, its formatter and the layer interface
- [`examples/planetaryscan/PlanetaryScanLayer.cpp`](../../examples/planetaryscan/PlanetaryScanLayer.cpp): Subscribing, launching tasks, storing results and drawing

---

## Running the Examples

The examples are built by default when Nodens is the top-level project (`ND_BUILD_EXAMPLES` is `ON`). From the Nodens repository root, inside the devcontainer:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run an example
./build/examples/circularwave3d/example-circularwave3d
./build/examples/planetaryscan/example-planetaryscan
```

To build only one, name its target: `cmake --build build --target example-planetaryscan`. To profile an example, configure with `-DCMAKE_BUILD_TYPE=Profiling` and connect the Tracy profiler; see [Profiling with Tracy](./profiling-with-tracy.md).
