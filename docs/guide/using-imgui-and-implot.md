# Using ImGui and ImPlot

Draw your user interface and plots from a layer, with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch), [ImPlot](https://github.com/epezent/implot) and [ImPlot3D](https://github.com/brenocq/implot3d).

> [!NOTE]
> **Mental model.** ImGui is immediate mode, like redrawing a whiteboard from scratch every frame. You describe the whole UI in code each frame, from your own variables, and nothing persists except what you store yourself. Nodens opens and closes the ImGui frame for you, so inside `OnImGuiRender` you only call `ImGui::`, `ImPlot::` and `ImPlot3D::` functions.
>
> This page goes from the plumbing to the pictures:
>
> 1. **What Nodens sets up.** [Setup](#what-nodens-sets-up)
> 2. **Draw a window.** [Drawing UI](#drawing-ui)
> 3. **Change the look.** [Themes](#themes)
> 4. **Plot data.** [2D plots](#2d-plots-implot) and [3D plots](#3d-plots-implot3d)
> 5. **Share the keyboard and mouse.** [Input](#input)
> 6. **Avoid the usual traps.** [Tips](#tips)

## What Nodens sets up

When `FApplicationConfig::Presentation` is `EPresentation::WindowImGUI`, `Application` creates an `ImGuiLayer` with a renderer for your graphics API and pushes it as an overlay. You never create it yourself. In the other two states there is no ImGui layer and `OnImGuiRender` is never called.

On attach, the `ImGuiLayer` creates the ImGui, ImPlot and ImPlot3D contexts, enables keyboard navigation and docking, applies the theme, and initializes the renderer for the chosen API. Multi-viewport is enabled only with the OpenGL backend; the default Vulkan backend runs ImGui in the main window.

Every frame, `ImGuiLayer::Begin()` starts the ImGui frame and creates a dockspace over the whole viewport, every layer's `OnImGuiRender` runs, and `ImGuiLayer::End()` renders the result. [Architecture Overview](./architecture-overview.md) shows where this sits in the loop.

## Drawing UI

Include the ImGui header in the global module fragment of your layer's implementation file, then draw in `OnImGuiRender`:

```cpp
module;

#include <imgui.h>

module MyApp.MyLayer;

import nodens;

void MyLayer::OnImGuiRender(Nodens::TimeStep ts)
{
    ImGui::Begin("My Window");
    ImGui::Text("Hello from MyLayer!");
    ImGui::Text("Frame time: %.3f ms", ts.GetMilliseconds());
    ImGui::End();
}
```

Every layer's `OnImGuiRender` runs once per frame in stack order, so each layer can own its own windows. The `TimeStep` is the same delta `OnUpdate` received. Read whether ImGui wants the keyboard or mouse from `ImGui::GetIO()` (`WantCaptureKeyboard`, `WantCaptureMouse`).

## Themes

`FApplicationConfig::Theme` selects the look at startup:

| `ETheme` | Result |
|---|---|
| `Dark` (default) | The Nodens dark theme: teal accents and layered elevation |
| `Light` | The Nodens light theme: deep teal accents and subtle contrast |
| `None` | No Nodens styling; ImGui's own dark colors |

```cpp
static inline const Nodens::FApplicationConfig appConfig = {
    .Name = "My Nodens Application",
    .Theme = Nodens::ETheme::Light,
};
```

The theme functions are public, so you can switch at runtime; they set both the ImGui and ImPlot styles:

```cpp
if (ImGui::Button("Light"))
    Nodens::ApplyDefaultLightTheme();
ImGui::SameLine();
if (ImGui::Button("Dark"))
    Nodens::ApplyDefaultDarkTheme();
```

To adjust single values, change `ImGui::GetStyle()` or `ImPlot::GetStyle()` in a layer's `OnAttach`. The theme is applied while the `Application` constructor runs, before your layers are pushed, so your overrides win.

## 2D plots (ImPlot)

Include `<implot.h>` and draw a plot inside an ImGui window. This is the `circularwave3d` example's 2D plot:

```cpp
#include <imgui.h>
#include <implot.h>

ImGui::Begin("ImPlot Example");
if (ImPlot::BeginPlot("Wave components", ImVec2(-1, -1)))
{
    ImPlot::SetupAxes("t", "x-y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
    ImPlot::PlotLine("Sine", t, xs, kNumberOfPoints);
    ImPlot::PlotLine("Cosine", t, ys, kNumberOfPoints);
    ImPlot::EndPlot();
}
ImGui::End();
```

- `ImVec2(-1, -1)` fills the available space; `SetupAxes` names the axes, and `ImPlotAxisFlags_AutoFit` makes them follow the data.
- Other plot types take the same kind of arrays: `PlotScatter` and `PlotBars` (both used by PlanetaryScan).
- **Real-time data:** keep the arrays in your layer, update them in `OnUpdate`, plot them in `OnImGuiRender`. If a task fills them on another thread, guard them with a mutex, as PlanetaryScan does.

## 3D plots (ImPlot3D)

Include `<implot3d.h>`. A 3D line takes three arrays; this is the same example's 3D plot:

```cpp
#include <implot3d.h>

ImGui::Begin("ImPlot3D Example");
if (ImPlot3D::BeginPlot("3D Circular Wave", ImVec2(-1, -1)))
{
    ImPlot3D::SetupAxes("t", "x", "y",
                        ImPlot3DAxisFlags_AutoFit,
                        ImPlot3DAxisFlags_AutoFit,
                        ImPlot3DAxisFlags_AutoFit);
    ImPlot3D::PlotLine("Wave", t, xs, ys, kNumberOfPoints);
    ImPlot3D::EndPlot();
}
ImGui::End();
```

It differs from ImPlot in having three axes and a rotatable camera; the [ImPlot3D documentation](https://github.com/brenocq/implot3d) lists the mouse controls.

## Input

By default the `ImGuiLayer` consumes mouse and keyboard events while ImGui wants them, so clicking a button does not also move your camera. Set `ShouldImGuiBlockInputs = false` in the application specification to pass everything through. The mechanism is in [Event System](./event-system.md).

## Tips

- **Keep `OnImGuiRender` cheap.** Compute in `OnUpdate` or in a task ([Concurrency and TaskSystem](./concurrency-and-tasksystem.md)) and only display the result here.
- **ImGui belongs to the main thread.** Never call `ImGui::` or `ImPlot::` from a task.
- **Window layouts persist** in `imgui.ini` in the working directory, because Nodens keeps ImGui's default. Delete the file to reset; the template and the Nodens repository already ignore it in version control.
- **Fonts and DPI.** Nodens loads no custom fonts and applies no DPI scaling. To load fonts, use `ImGui::GetIO().Fonts` in a layer's `OnAttach`.
- **Docking.** A full-viewport dockspace is always present, so you can dock windows by dragging their title bars.

## See Also

- [Layer System](./layer-system.md): when `OnImGuiRender` runs.
- [Creating a Custom Layer](./creating-a-custom-layer.md): a layer with UI.
- [Examples Guide](./examples-guide.md): `circularwave3d` uses ImPlot and ImPlot3D.
