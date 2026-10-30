# Creating a Custom Layer

Build one layer from scratch: a window of live frame statistics that pauses when you press Space.

> [!NOTE]
> **Mental model.** A layer is a class you hand over to Nodens, and from then on Nodens calls you. You write a C++ module that exports one class derived from `Nodens::ILayer`, push an instance, and override only the callbacks you care about.
>
> The tutorial has five steps:
>
> 1. **Declare the class.** [Step 1](#step-1-declare-the-module-interface)
> 2. **Fill in the callbacks.** [Step 2](#step-2-implement-the-layer)
> 3. **Tell CMake about the files.** [Step 3](#step-3-register-with-cmake)
> 4. **Push the layer.** [Step 4](#step-4-push-the-layer)
> 5. **Build and run.** [Step 5](#step-5-build-and-run)
>
> You need a working Nodens application, such as one made from the [template](https://github.com/eldritch-codex/template-nodens-app) ([Getting Started](./getting-started.md)), and a glance at the [Layer System](./layer-system.md). The steps add a layer interface and its implementation file to your own project, and change its CMakeLists and its main source file.

## Step 1: Declare the Module Interface

A layer lives in a C++ module. The interface file (`.cppm`) declares the module and the class; the implementation goes in a separate `.cpp` file.

```cpp
// src/FrameStatsLayer.cppm
export module MyApp.FrameStatsLayer;

import nodens;

export class FrameStatsLayer : public Nodens::ILayer
{
public:
    FrameStatsLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Nodens::TimeStep ts) override;
    void OnImGuiRender(Nodens::TimeStep ts) override;
    void OnInputEvent(Nodens::RoutedInputEvent& event) override;

private:
    float m_SmoothedFrameMs{0.0f}; // Moving average of the frame time
    float m_Elapsed{0.0f};         // Seconds the layer has been running
    bool m_Paused{false};
};
```

- `export module MyApp.FrameStatsLayer;` names the module. Any dotted name works; `MyApp.` is a convention that keeps your modules apart from Nodens' own.
- `import nodens;` is the only import you need for the framework. It brings in `ILayer`, `TimeStep`, `RoutedInputEvent`, the logger and the rest.
- You override only the callbacks you need. They are all optional.

## Step 2: Implement the Layer

The implementation file starts with a global module fragment so it can include the ImGui header, then declares which module it implements.

```cpp
// src/FrameStatsLayer.cpp
module;

#include <imgui.h>

module MyApp.FrameStatsLayer;

import nodens;

FrameStatsLayer::FrameStatsLayer() : ILayer("FrameStats")
{
}

void FrameStatsLayer::OnAttach()
{
    Nodens::ClientLogger().info("{} attached", GetName());
}

void FrameStatsLayer::OnDetach()
{
    Nodens::ClientLogger().info("{} detached after {:.1f} s", GetName(), m_Elapsed);
}

void FrameStatsLayer::OnUpdate(Nodens::TimeStep ts)
{
    if (m_Paused)
        return;

    m_Elapsed += ts.GetSeconds();

    // Each frame moves the average 5% of the way toward the latest frame time.
    constexpr float kSmoothing{0.05f};
    m_SmoothedFrameMs += kSmoothing * (ts.GetMilliseconds() - m_SmoothedFrameMs);
}

void FrameStatsLayer::OnImGuiRender(Nodens::TimeStep)
{
    ImGui::Begin("Frame Stats");

    const float fps = m_SmoothedFrameMs > 0.0f ? 1000.0f / m_SmoothedFrameMs : 0.0f;
    ImGui::Text("Frame time: %.2f ms (%.0f FPS)", m_SmoothedFrameMs, fps);
    ImGui::Text("Running for: %.1f s", m_Elapsed);

    ImGui::Checkbox("Paused (Space toggles)", &m_Paused);
    if (ImGui::Button("Reset"))
    {
        m_Elapsed = 0.0f;
        m_SmoothedFrameMs = 0.0f;
    }

    ImGui::End();
}

void FrameStatsLayer::OnInputEvent(Nodens::RoutedInputEvent& event)
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

- **`OnAttach` and `OnDetach`** log through `Nodens::ClientLogger()`, the application's logger (named `APP`). `OnDetach` runs when the application shuts down.
- **`OnUpdate`** receives the frame's `TimeStep`. The smoothing line is an exponential moving average: instead of showing each frame's time, which jitters, it moves a fixed fraction toward the new sample.
- **`OnImGuiRender`** draws with the ImGui API directly; the framework has already opened the ImGui frame. See [Using ImGui and ImPlot](./using-imgui-and-implot.md).
- **`OnInputEvent`** receives a `RoutedInputEvent`. The `InputEventDispatcher` runs your lambda only when the event holds a `KeyPressed`. The lambda's return value decides whether the event is consumed: `true` stops it reaching layers below this one. Returning `false` for any other key leaves it free for them.

## Step 3: Register with CMake

The interface file is a module, so it goes in a `CXX_MODULES` file set. The implementation file is an ordinary source. Update the app's CMakeLists:

```cmake
add_executable(NodensApp src/main.cpp src/FrameStatsLayer.cpp)

target_sources(NodensApp
    PRIVATE
        FILE_SET app_modules TYPE CXX_MODULES
        FILES src/FrameStatsLayer.cppm
)

target_link_libraries(NodensApp PRIVATE Nodens::Nodens)
```

Each additional layer adds its implementation to `add_executable` and its interface to the `FILES` list. The development image uses the Ninja generator by default, which CMake needs to work out the order in which modules must be built.

## Step 4: Push the Layer

Import your module in the application's main source file and push the layer from its constructor:

```cpp
import nodens;
import MyApp.FrameStatsLayer;

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
        PushLayer(new FrameStatsLayer());
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

Two rules apply to `PushLayer`:

- **Ownership.** Pass a pointer from `new` and do not delete it. The `LayerStack` owns the layer and deletes it when the application shuts down.
- **Order.** Layers update and draw in the order you push them, and receive input in the reverse order. With several layers, the last one pushed sees input first. All of them sit below the `ImGuiLayer` overlay. See [Layer System](./layer-system.md).

## Step 5: Build and Run

Build and run from inside the devcontainer:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target NodensApp
./build/NodensApp
```

You should see:

- A window with a **Frame Stats** panel showing the smoothed frame time, an FPS figure and a running timer.
- `FrameStats attached` in the console when the app starts, and `FrameStats detached after N s` when you close the window.
- Pressing **Space** pauses the timer and ticks the **Paused** checkbox. Pressing it again resumes. While an ImGui text field has keyboard focus, the `ImGuiLayer` consumes the key first and your layer does not see it, which is the intended input blocking (see [Event System](./event-system.md)).

## Where to go next

- Understand the callbacks and ordering: [Layer System](./layer-system.md).
- Handle more input: [Event System](./event-system.md).
- Move work off the main thread with the [Concurrency and TaskSystem](./concurrency-and-tasksystem.md).
- Send results between layers with the `EventBus`, as in the PlanetaryScan example ([Examples Guide](./examples-guide.md)).
- Draw plots with ImPlot ([Using ImGui and ImPlot](./using-imgui-and-implot.md)).
