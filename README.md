<h1 align="center">Nodens</h1>

<h3 align="center">An experimental C++23 framework for interactive desktop applications.</h3>

<p align="center">
  <a href="https://github.com/eldritch-codex/nodens/releases"><img src="https://img.shields.io/github/v/release/eldritch-codex/nodens?style=flat-square&amp;label=release" alt="Latest release"></a>
  <a href="https://github.com/eldritch-codex/nodens/actions/workflows/ci.yml"><img src="https://img.shields.io/github/actions/workflow/status/eldritch-codex/nodens/ci.yml?branch=main&amp;style=flat-square&amp;label=CI" alt="CI status"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue?style=flat-square" alt="C++23">
  <img src="https://img.shields.io/badge/CMake-3.30-blue?style=flat-square" alt="CMake 3.30">
  <img src="https://img.shields.io/badge/platform-Linux-blue?style=flat-square" alt="Platform: Linux">
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/license-MIT-yellow?style=flat-square" alt="License: MIT"></a>
</p>

<p align="center">
  <a href="docs/guide/getting-started.md">Getting Started</a> ·
  <a href="docs/guide/README.md">Guide</a> ·
  <a href="https://github.com/eldritch-codex/template-nodens-app">Application Template</a>
</p>

> **Status:** Nodens is experimental and under active development. APIs change freely, and a breaking change bumps the minor version.
>
> **Limitations:** Arch Linux is the only validated host, so Windows and macOS are unsupported. A release is the tagged source plus its changelog section: no install rules, binary packages or hosted API documentation, and the test suite is not written yet.

## Introduction

**Nodens** unifies third-party libraries ([ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot), [ImPlot3D](https://github.com/brenocq/implot3d), [Tracy](https://github.com/wolfpld/tracy)) with a custom core that integrates four systems:

- **GLFW Windows** with OpenGL and Vulkan backends. Nodens exposes no renderer API of its own, since renderers such as [Nyar](https://github.com/eldritch-codex/nyar) borrow the window's graphics backend.
- **Layer System** that governs the application lifetime, execution flow, render priority and input event routing (see [Layer System](docs/guide/layer-system.md)).
- **Event System** with InputEvent routing through the LayerStack, plus a thread-safe pub/sub EventBus with immediate and queued dispatch (see [Event System](docs/guide/event-system.md)).
- **Task System** for multithreaded task execution (see [Concurrency and TaskSystem](docs/guide/concurrency-and-tasksystem.md)).

The window and the ImGui overlay are optional: the same loop services layers and events with no window at all, paced by an explicit policy (see [Application Loop](docs/guide/application-loop.md)).

Nodens follows a **module-first architecture**: public APIs are module interfaces (`.cppm`), third-party headers are isolated on the global fragment module, and consumers simply `import nodens;`. A project takes Nodens from CMake with `FetchContent`.

The framework compiles into a single **static library** linked to your application, so the result is one portable executable.

Documentation: [`docs/`](docs/README.md) indexes it all. The [guide](docs/guide/README.md) holds the tutorials, architecture guides and how-tos, including [Building and Toolchain](docs/guide/building-and-toolchain.md) and [Profiling with Tracy](docs/guide/profiling-with-tracy.md); [`docs/dev`](docs/dev/naming.md) holds the design notes, and the root holds [branching](docs/BRANCHING.md) and [committing](docs/COMMITTING.md). The API reference is Doxygen output built with `cmake --build build --target nodens-docs`, not yet hosted.

## A minimal application

An application subclasses `Nodens::Application`, pushes its layers, and calls `Run()`. The window, ImGui overlay, task system and event bus come with it.

```cpp
#include <imgui.h>

import nodens;

class HelloLayer : public Nodens::ILayer {
public:
    HelloLayer() : ILayer("Hello") {}

    void OnUpdate(Nodens::TimeStep ts) override {
        m_Seconds += ts.GetSeconds();
    }

    void OnImGuiRender(Nodens::TimeStep ts) override {
        ImGui::Begin("Hello, Nodens");
        ImGui::Text("Running for %.1f s", m_Seconds);
        ImGui::End();
    }

private:
    float m_Seconds = 0.f;
};

class HelloApp : public Nodens::Application {
public:
    HelloApp() : Application({.Name = "Hello Nodens"}) {
        PushLayer(new HelloLayer());
    }
};

int main() {
    Nodens::InitializeLoggers();
    HelloApp{}.Run();
}
```

Layers receive `OnUpdate` and `OnImGuiRender` every frame, plus events routed down the stack. `FApplicationConfig` sets the window size, graphics API, theme and whether ImGui is enabled; the other layer callbacks are in [Layer System](docs/guide/layer-system.md). Building this app needs the toolchain steps in [Getting Started](docs/guide/getting-started.md), or start from the [application template](https://github.com/eldritch-codex/template-nodens-app).

## Example Applications

Examples live in `examples/`:

| Example | Demonstrates |
|---------|-------------|
| **`circularwave3d`** | Immediate mode GUI, live 2D and 3D plotting with ImPlot and ImPlot3D |
| **`planetaryscan`** | Pub/sub event bus, multithreaded task system with thread-safe data collection, live scatter plot |

### Profiling a multithreaded workload

https://github.com/user-attachments/assets/4d345eb7-46c5-4360-a7f0-55466ec753ff

### Applications from earlier Nodens versions, used in numerical-simulation research

https://github.com/user-attachments/assets/3025146c-d49b-4a10-84ea-3dcfa74ac59e

https://github.com/user-attachments/assets/28be4bea-d0bd-46aa-bd1c-004daedd4243

## Dependencies

Dependencies are fetched at configure time with CMake `FetchContent` and built from source, except GLAD, which is vendored in `vendor/glad`.

| Library | Description | Version |
| :-- | :-- | :-- |
| **[GLAD](https://glad.dav1d.de/)** | OpenGL function loader | v0.1.35 |
| **[GLFW](https://www.glfw.org/)** | Window, context and input | 3.4 |
| **[ImGui](https://github.com/ocornut/imgui)** | Immediate mode GUI | v1.92.9b-docking |
| **[ImPlot](https://github.com/epezent/implot)** | 2D plotting for ImGui | v1.1 |
| **[ImPlot3D](https://github.com/brenocq/implot3d)** | 3D plotting for ImGui | v0.4 |
| **[spdlog](https://github.com/gabime/spdlog)** | Logging | v1.17.0 |
| **[Tracy](https://github.com/wolfpld/tracy)** | Frame and GPU profiler | v0.14.1 |

## Acknowledgements

A special thanks to **TheCherno**, whose [Game Engine series](http://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT) inspired the foundational architecture of Nodens.

Nodens began as my undergraduate Computer Graphics project at UFMG in 2022, and early versions powered [boids](https://github.com/renanbomtempo/boids) and [galaxians](https://github.com/renanbomtempo/galaxians). The origins of the project are told in [this discussion](https://github.com/orgs/eldritch-codex/discussions/1).

In September 2026 I started experimenting with LLM coding agents (Claude Code, ZedAI and Pi) and even decided to develop my own: [Armitage](https://eldritchcodex.com/armitage/). I use them mostly to learn new stuff, but also they help with documentation, CI/CD and general project upkeep. Commits carry no co-author or AI attribution trailers, since I am responsible for every commit.

## Suggested reading

Books worth reading if you work on Nodens or build on it:

- Jason Gregory, *Game Engine Architecture*, Third edition (Boca Raton London New York: CRC Press, Taylor & Francis Group, 2019). Engine architecture.
- Ivor Horton and Peter Van Weert, *Beginning C++23: From Beginner to Pro* (Berkeley, CA: Apress, 2023). https://doi.org/10.1007/978-1-4842-9343-0. Modern C++ up to C++23, which Nodens targets.

## License

Copyright (c) 2026 EldritchCodex. The project and its entire history are licensed under the MIT License.
