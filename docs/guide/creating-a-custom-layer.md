# Creating a Custom Layer

Build one layer from scratch: a panel of live iteration statistics that pauses when Space is pressed.

> [!NOTE]
> **Mental model.** A layer is a class you hand to Nodens, and from then on Nodens calls it. You write a C++ module that exports one class derived from `Nodens::ILayer`, push an instance, and override only the callbacks you need.

## Problem

A layer is the unit that receives the loop's callbacks, but making one run touches four things, and each fails differently when it is wrong.

A class needs a module interface and an implementation file. The implementation must include the ImGui header outside the module, which means a global module fragment in a fixed place. The build needs the interface named as a module, not as an ordinary source. And the layer must be pushed as a raw pointer that the framework then owns.

Get the first three wrong and the build fails with an error that names the symptom rather than the cause. Get the fourth wrong and the application double-frees a layer or leaks one.

## How it works

### The three files

| File | Holds | Why it is separate |
|---|---|---|
| `MyLayer.cppm` | `export module`, the class, its members | The interface is what other modules import |
| `MyLayer.cpp` | The callback bodies, and the ImGui include in the global module fragment | Third-party headers cannot enter a module's purview |
| `CMakeLists.txt` | The implementation as a source, the interface in a `CXX_MODULES` file set | The build must know which file is a module |

### The five callbacks

| Callback | Runs | Use it for |
|---|---|---|
| `OnAttach()` | Once, inside `PushLayer` | Resources, subscriptions, fonts |
| `OnDetach()` | Once, at shutdown | Releasing what `OnAttach` took |
| `OnUpdate(TimeStep)` | Once per step | State and simulation |
| `OnImGuiRender(TimeStep)` | Once per iteration, when there is an overlay | Drawing |
| `OnInputEvent(RoutedInputEvent&)` | Per input event, back to front | One-shot reactions |

All five are optional. [Layer System](./layer-system.md) explains the order they run in.

### Pushing it

`PushLayer(new MyLayer())` from the application constructor. Two rules come with it: the `LayerStack` owns the pointer and deletes it at shutdown, so never delete it yourself, and the push order decides update order, with the reverse deciding input order.

## What still goes wrong

**The interface left out of the build.** A `.cppm` listed as an ordinary source, or forgotten, produces a module that nothing can import.

**The layer deleted by hand.** The stack owns it and deletes it at shutdown, so a manual `delete` frees it twice.

**An include inside the module.** Writing `#include <imgui.h>` after `module MyApp.MyLayer;` puts a third-party header inside the module purview, which the language forbids for this case.

**A key that toggles itself many times.** Holding Space produces one `KeyPressed` with `RepeatCount` zero followed by repeats with a higher count, so a handler that ignores the count flickers the state.

**A key that never arrives.** While an ImGui text field has focus, the overlay consumes the keyboard first.

**An update that runs more than expected.** In fixed pacing `OnUpdate` runs once per step, and an iteration may run several steps.

## Remedies

**Name the interface in a `CXX_MODULES` file set.** That is the only place the build learns the file is a module.

**Never delete a layer you pushed.** Pass a pointer from `new` and let the stack handle the rest.

**Put the include in the global module fragment,** above `module MyApp.MyLayer;`.

**Check `RepeatCount`.** Consume the event only when it is zero, so one press toggles the state once.

**Read the input policy.** `ShouldImGuiBlockInputs` decides whether the overlay keeps keys that ImGui wants, and [Event System](./event-system.md) explains when to change it.

**Treat `OnUpdate` as per step.** Multiply by `ts`, and count steps rather than iterations.

## A worked case

### Step 1: declare the module interface

```cpp
// src/IterationStatsLayer.cppm
export module MyApp.IterationStatsLayer;

import nodens;

export class IterationStatsLayer : public Nodens::ILayer
{
public:
    IterationStatsLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Nodens::TimeStep ts) override;
    void OnImGuiRender(Nodens::TimeStep ts) override;
    void OnInputEvent(Nodens::RoutedInputEvent& event) override;

private:
    float m_SmoothedIterationMs{0.0f}; // Moving average of one iteration's time
    float m_Elapsed{0.0f};             // Seconds the layer has been running
    bool m_Paused{false};
};
```

`export module MyApp.IterationStatsLayer;` names the module, and any dotted name works; the `MyApp.` prefix is a convention that keeps your modules apart from the framework's. `import nodens;` is the only import needed: it brings in `ILayer`, `TimeStep`, `RoutedInputEvent` and the logger.

### Step 2: implement the layer

```cpp
// src/IterationStatsLayer.cpp
module;

#include <imgui.h>

module MyApp.IterationStatsLayer;

import nodens;

IterationStatsLayer::IterationStatsLayer() : ILayer("IterationStats")
{
}

void IterationStatsLayer::OnAttach()
{
    Nodens::ClientLogger().info("{} attached", GetName());
}

void IterationStatsLayer::OnDetach()
{
    Nodens::ClientLogger().info("{} detached after {:.1f} s", GetName(), m_Elapsed);
}

void IterationStatsLayer::OnUpdate(Nodens::TimeStep ts)
{
    if (m_Paused)
        return;

    m_Elapsed += ts.GetSeconds();

    // Each update moves the average 5% of the way toward the latest measurement.
    constexpr float kSmoothing{0.05f};
    m_SmoothedIterationMs += kSmoothing * (ts.GetMilliseconds() - m_SmoothedIterationMs);
}

void IterationStatsLayer::OnImGuiRender(Nodens::TimeStep)
{
    ImGui::Begin("Iteration Stats");

    const float perSecond = m_SmoothedIterationMs > 0.0f ? 1000.0f / m_SmoothedIterationMs : 0.0f;
    ImGui::Text("Iteration time: %.2f ms (%.0f per second)", m_SmoothedIterationMs, perSecond);
    ImGui::Text("Running for: %.1f s", m_Elapsed);

    ImGui::Checkbox("Paused (Space toggles)", &m_Paused);
    if (ImGui::Button("Reset"))
    {
        m_Elapsed = 0.0f;
        m_SmoothedIterationMs = 0.0f;
    }

    ImGui::End();
}

void IterationStatsLayer::OnInputEvent(Nodens::RoutedInputEvent& event)
{
    Nodens::InputEventDispatcher dispatcher(event);
    dispatcher.Dispatch<Nodens::InputEvents::KeyPressed>(
        [this](Nodens::InputEvents::KeyPressed& e)
        {
            // Ignore auto-repeat so holding Space does not flicker the state.
            if (e.KeyCode != static_cast<int>(Nodens::EKeyboardKey::Space) || e.RepeatCount > 0)
                return false;

            m_Paused = !m_Paused;
            return true; // Consume the event so layers below this one never see it.
        });
}
```

What each callback does here:

- **`OnAttach` and `OnDetach`** log through `Nodens::ClientLogger()`, the application's logger, named `APP`. `OnDetach` runs at shutdown.
- **`OnUpdate`** receives the step's `TimeStep`. The smoothing line is an exponential moving average: instead of showing one measurement, which jitters, it moves a fixed fraction toward the newest one.
- **`OnImGuiRender`** draws with the ImGui API directly, because the overlay has already opened the frame. See [Using ImGui and ImPlot](./using-imgui-and-implot.md).
- **`OnInputEvent`** receives a `RoutedInputEvent`, and the dispatcher runs the lambda only for a `KeyPressed`. The return value decides consumption: `true` stops the event reaching the layers below.

### Step 3: register with CMake

```cmake
add_executable(NodensApp src/main.cpp src/IterationStatsLayer.cpp)

target_sources(NodensApp
    PRIVATE
        FILE_SET app_modules TYPE CXX_MODULES
        FILES src/IterationStatsLayer.cppm
)

target_link_libraries(NodensApp PRIVATE Nodens::Nodens)
```

Each added layer contributes its implementation to `add_executable` and its interface to the `FILES` list. The development image uses the Ninja generator, which CMake needs to work out the order in which modules are built.

### Step 4: push the layer

```cpp
import nodens;
import MyApp.IterationStatsLayer;

class MyApp : public Nodens::Application
{
public:
    static inline const Nodens::FApplicationConfig appConfig = {
        .Name = "My Nodens Application",
        .WindowWidth = 1280,
        .WindowHeight = 720,
        .Presentation = Nodens::EPresentation::WindowImGUI,
    };

    MyApp() : Application(appConfig)
    {
        PushLayer(new IterationStatsLayer());
    }
};

int main()
{
    Nodens::InitializeLoggers();

    auto app = MyApp();
    app.Run();

    return 0;
}
```

The layer sits below the `ImGuiLayer` overlay, because the application pushed the overlay first. It therefore draws under the overlay's dockspace and receives input after it. [Layer System](./layer-system.md) has the order table.

### Step 5: build and run

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target NodensApp
./build/NodensApp
```

What you should see:

- A window with an **Iteration Stats** panel showing the smoothed iteration time, the rate per second and a running timer.
- `IterationStats attached` in the console at start, and `IterationStats detached after N s` when the window closes.
- Pressing Space pauses the timer and ticks the checkbox; pressing it again resumes.
- While an ImGui text field has keyboard focus, the overlay consumes the key first, so the layer does not see it. That is the intended blocking, explained in [Event System](./event-system.md).

## Choosing

- **Which callbacks?** Override only what the layer needs. An empty body costs a virtual call per step and nothing else.
- **Layer or overlay?** `PushLayer` for application content, `PushOverlay` for a debug panel or an input filter that must run first.
- **Module or header?** Nodens is module-first, so a layer used by another module is an exported module interface. A layer private to one executable may be a plain header, but the CMake file set differs and the examples use modules.

## Rules

1. One class per layer, exported from its own module.
2. The ImGui include goes above `module ...;`, never below it.
3. Push a pointer from `new`, and never delete it.
4. Consume an input event only when you mean to stop it reaching the layers below.
5. Do state in `OnUpdate`, drawing in `OnImGuiRender`.

## See Also

- [Layer System](./layer-system.md): the callbacks and their order.
- [Event System](./event-system.md): input events and the bus.
- [Concurrency and TaskSystem](./concurrency-and-tasksystem.md): moving work off the main thread.
- [Using ImGui and ImPlot](./using-imgui-and-implot.md): plots inside a layer.
