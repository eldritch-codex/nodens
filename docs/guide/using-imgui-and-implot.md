# Using ImGui and ImPlot

Draw the interface and the plots from a layer, with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch), [ImPlot](https://github.com/epezent/implot) and [ImPlot3D](https://github.com/brenocq/implot3d).

> [!NOTE]
> **Mental model.** ImGui is immediate mode: the whole interface is described in code every iteration, and nothing persists except what you store yourself. Nodens opens and closes the ImGui frame around your drawing, so inside `OnImGuiRender` you only call `ImGui::`, `ImPlot::` and `ImPlot3D::` functions.

## Problem

An immediate-mode interface needs plumbing that has nothing to do with the interface itself. Something must create the ImGui, ImPlot and ImPlot3D contexts, pick a renderer for the graphics API in use, open the frame each iteration and close it, and decide who owns the keyboard and mouse when the interface and the application both want them at the same time.

Done ad hoc in each application, that plumbing is repeated, and the last part is where it breaks: a click meant for the camera also lands on a button, or a text field stops receiving keystrokes because a layer below it consumed them first.

Nodens provides the plumbing once, in one overlay layer, and gives the application four things to set: the theme, the input policy, the drawing, and nothing else.

## How it works

### What Nodens sets up

When `FApplicationConfig::Presentation` is `EPresentation::WindowImGUI`, `Application` creates an `ImGuiLayer` with a renderer for the graphics API and pushes it as an overlay. You never create it yourself. In the other two presentation states there is no overlay and `OnImGuiRender` is never called.

On attach, the overlay creates the ImGui, ImPlot and ImPlot3D contexts, enables keyboard navigation and docking, applies the theme, and initializes the renderer for the chosen API. Multi-viewport is enabled only with the OpenGL backend; the default Vulkan backend runs ImGui inside the main window.

Every iteration, `ImGuiLayer::Begin()` starts the ImGui frame and creates a dockspace over the whole viewport, every layer's `OnImGuiRender` runs once, and `ImGuiLayer::End()` renders the result. [Architecture Overview](./architecture-overview.md) shows where that sits in the loop.

### Drawing

Include the ImGui header in the global module fragment of the implementation file, then draw in `OnImGuiRender`:

```cpp
module;

#include <imgui.h>

module MyApp.MyLayer;

import nodens;

void MyLayer::OnImGuiRender(Nodens::TimeStep ts)
{
    ImGui::Begin("My Window");
    ImGui::Text("Hello from MyLayer!");
    ImGui::Text("Iteration time: %.3f ms", ts.GetMilliseconds());
    ImGui::End();
}
```

Each layer draws its own windows, in stack order, once per iteration. The `TimeStep` is the same step `OnUpdate` received.

### Themes

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

The theme functions are public, so a layer can switch at runtime. They set both the ImGui and ImPlot styles:

```cpp
if (ImGui::Button("Light"))
    Nodens::ApplyDefaultLightTheme();
ImGui::SameLine();
if (ImGui::Button("Dark"))
    Nodens::ApplyDefaultDarkTheme();
```

The theme is applied while the `Application` constructor runs, before your layers are pushed, so a change made in `OnAttach` or later wins.

### 2D plots with ImPlot

Include `<implot.h>` and draw a plot inside an ImGui window. This is the `circularwave3d` example's plot:

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

`ImVec2(-1, -1)` fills the available space, `SetupAxes` names the axes and `ImPlotAxisFlags_AutoFit` makes them follow the data. Other plot types take the same kind of arrays: `PlotScatter` and `PlotBars`, both used by PlanetaryScan.

### 3D plots with ImPlot3D

Include `<implot3d.h>`. A 3D line takes three arrays:

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

## What still goes wrong

**Drawing from a worker.** Every `ImGui::`, `ImPlot::` and `ImPlot3D::` call belongs to the main thread.

**Logic placed in `OnImGuiRender`.** That callback runs only in the overlay state, so state changes made there vanish in a headless run.

**An expensive interface.** Drawing runs once per iteration on the main thread, and a plot rebuilt from scratch each iteration costs the same as the simulation it shows.

**Input that never arrives.** The overlay consumes mouse and keyboard events while ImGui wants them, so a layer that expects every click sees none.

**Input that arrives when it should not.** With `ShouldImGuiBlockInputs = false`, a click on a button also reaches the camera behind it.

**A layout that reappears.** ImGui writes `imgui.ini` in the working directory, so windows reopen where they were closed.

**Fonts that look wrong or too small.** Nodens loads no custom font and applies no DPI scaling.

## Remedies

**Keep every ImGui call on the main thread.** A task computes and reports through a queued event or a future, and the layer draws the result.

**Change state in `OnUpdate`, draw in `OnImGuiRender`.** The split is what keeps the logic alive without an overlay.

**Compute in `OnUpdate` and only display here.** Keep the arrays in the layer, update them per step, and let the plot read them.

**Read what ImGui wants.** `ImGui::GetIO()` reports `WantCaptureMouse` and `WantCaptureKeyboard`, and a layer can check them before acting on input.

**Unblock input deliberately.** Set `ShouldImGuiBlockInputs = false` when a layer must see input that ImGui captures. The mechanism is in [Event System](./event-system.md).

**Delete `imgui.ini` to reset a layout,** or dock the windows again and keep it. The template and the Nodens repository ignore it in version control.

**Load a font in `OnAttach`.** Use `ImGui::GetIO().Fonts` there; it is the only place the context is already created and the layers are not yet drawing.

## A worked case

`circularwave3d` keeps three arrays of a thousand floats and draws them twice:

| Step | Callback | What happens |
|---|---|---|
| 1 | `OnUpdate(ts)` | Each sample's time advances, and the sine and cosine are recomputed |
| 2 | `OnImGuiRender(ts)` | Two ImGui windows open, one with an ImPlot line pair, one with an ImPlot3D curve over the same arrays |
| 3 | `OnImGuiRender(ts)` | Both plots read the arrays the step just wrote, so the picture is never one step behind |

The order matters: because every `OnUpdate` finishes before any `OnImGuiRender` starts, the plots always show the state this iteration produced. A plot that also wrote to the arrays would invert the order and show stale data.

If a task fills those arrays instead, the layer guards them with a mutex while plotting, as PlanetaryScan does.

## Choosing

- **ImPlot or ImPlot3D?** Two axes for time series and scatter, three for a curve or a surface. Both take arrays you own.
- **Theme at startup or at runtime?** Set `Theme` in the config for the usual case; switch with the theme functions only when the user asks for it.
- **Dock or float?** A full-viewport dockspace is always present, so windows can be dragged and docked without further setup.

## Rules

1. Every ImGui, ImPlot and ImPlot3D call runs on the main thread.
2. Draw in `OnImGuiRender`, change state in `OnUpdate`.
3. Read `WantCaptureMouse` and `WantCaptureKeyboard` before acting on input in a layer.
4. Load fonts in `OnAttach`, once.
5. Never create the overlay yourself; the application pushes it.

## See Also

- [Layer System](./layer-system.md): when `OnImGuiRender` runs and in which order.
- [Creating a Custom Layer](./creating-a-custom-layer.md): a layer with its own panel.
- [Examples Guide](./examples-guide.md): `circularwave3d` uses ImPlot and ImPlot3D.
