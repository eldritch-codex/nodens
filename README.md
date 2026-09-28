[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT) 
![Supported platforms: Linux ](https://img.shields.io/badge/Supported%20platforms-Linux-blue.svg?style=flat-square)

> ⚠️ **Development Status:** Nodens is currently in **active, experimental development** with no stable release. It serves primarily as a learning ground and playground for exploring game engine architecture and modern C++20/23 features. APIs are highly subject to change.

# Introduction

**Nodens** is an experimental C++23 framework designed for rapidly developing interactive desktop applications with an optional immediate mode graphical user interface. 

It unifies excellent third-party libraries (e.g. [ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot), [ImPlot3D](https://github.com/brenocq/implot3d), [Tracy](https://github.com/wolfpld/tracy)) with a custom core that integrates 4 main systems:
- **GLFW Windows** with both OpenGL and Vulkan backends.
- **Layer System** that governs the application lifetime and execution flow, visual rendering priority and input event routing (see [wiki](https://github.com/EldritchCodex/Nodens/wiki/Layer-System));
- **Event System** with InputEvent routing through the LayerStack and a pub/sub thread-safe EventBus for both immediate and queued dispatch of custom event types (see [wiki](https://github.com/EldritchCodex/Nodens/wiki/Event-System));
- **Job System** for multithreaded task execution (see [wiki](https://github.com/EldritchCodex/Nodens/wiki/Concurrency-and-JobSystem)).

Nodens follows a **module-first architecture** where public APIs are module interfaces (`.cppm`), third-party headers are isolated on the global fragment module, and consumers simply `import nodens;`.

The framework compiles into a single **static library** that is linked to your application, ensuring the final product is a single portable executable to streamline distribution.

# Documentation

| Resource | Description |
|----------|-------------|
| **[Wiki](https://github.com/EldritchCodex/Nodens/wiki)** | Tutorials, architecture guides, and how-to articles |
| **API Reference** | Doxygen-generated class and function documentation - *not yet hosted; build locally with `cmake --build build --target nodens-docs`* |

# Quick Start

> 📖 For build, integration, and development-environment instructions, see the **[Getting Started](https://github.com/EldritchCodex/Nodens/wiki/Getting-Started)** wiki page.

> ⚠️ **Platform limitation:** The supported and validated development environment is based on Arch Linux and currently builds Nodens for **Linux only**. Native Windows and macOS builds are not currently supported or validated.

For new applications, start from the [Nodens Application Template](https://github.com/EldritchCodex/Template-NodensApp). Nodens can also be consumed from CMake with `FetchContent`; the wiki covers framework development, local checkouts, and native Linux builds.

Build configurations and their options are documented in the wiki, under [Building and Toolchain](https://github.com/EldritchCodex/Nodens/wiki/Building-and-Toolchain) and [Profiling with Tracy](https://github.com/EldritchCodex/Nodens/wiki/Profiling-with-Tracy).

## Development Environment

Nodens includes a Linux Dev Container using
`ghcr.io/eldritch-codex/arch-dev:main`. Open the repository in Zed or
Visual Studio Code, then reopen it in the Dev Container.

NVIDIA hosts need the host NVIDIA driver and NVIDIA Container Toolkit before
using the default GPU configuration. Intel/AMD hosts should select the
alternative `/dev/dri` configuration in `.devcontainer/devcontainer.json`.

See [Getting Started](https://github.com/EldritchCodex/Nodens/wiki/Getting-Started)
for host setup, toolkit installation, GPU configuration, and build instructions.

# Example Applications

Nodens includes several examples in the `examples/` directory:

| Example | Demonstrates |
|---------|-------------|
| **`circularwave3d`** | Immediate mode GUI, real-time 2D/3D plotting with ImPlot and ImPlot3D |
| **`planetaryscan`** | Pub/Sub event bus, multi-threaded job system with thread-safe data collection, live scatter plot visualization |

> 📖 For detailed walkthroughs of each example, see the **[Examples Guide](https://github.com/EldritchCodex/Nodens/wiki/Examples-Guide)** on the wiki.

##### Multithreaded Workload Profiling Showcase
https://github.com/user-attachments/assets/4d345eb7-46c5-4360-a7f0-55466ec753ff


# Other Applications Showcase

Applications I used in my own research on numerical simulations using earlier versions of Nodens.

https://github.com/user-attachments/assets/3025146c-d49b-4a10-84ea-3dcfa74ac59e

https://github.com/user-attachments/assets/28be4bea-d0bd-46aa-bd1c-004daedd4243


# Dependencies

All dependencies are resolved automatically at configure time via CMake `FetchContent` and are downloaded and built from source (except from glad which is included in the `vendor` folder). 

| Library                                             | Description                                               | Version            | 
| :-------------------------------------------------- | :-------------------------------------------------------- | :----------------- | 
| **[GLAD](https://glad.dav1d.de/)**                  | OpenGL function loader.                                   | v0.1.35            | 
| **[GLFW](https://www.glfw.org/)**                   | Cross-platform for window, context, and input management. | 3.4                | 
| **[ImGui](https://github.com/ocornut/imgui)**       | Bloat-free Immediate Mode GUI.                            | v1.92.9b-docking |
| **[ImPlot](https://github.com/epezent/implot)**     | 2D data plotting extension for ImGui.                     | v1.0               | 
| **[ImPlot3D](https://github.com/brenocq/implot3d)** | 3D data plotting extension for ImGui.                     | v0.4               | 
| **[spdlog](https://github.com/gabime/spdlog)**      | Fast, header-only/compiled logging library.               | v1.17.0            | 
| **[Tracy](https://github.com/wolfpld/tracy)**       | Real-time frame and GPU profiler.                         | v0.14.1 |

# Acknowledgements

A special thanks to **TheCherno**. The foundational architecture of Nodens was heavily inspired by the early episodes of his excellent [Game Engine series](http://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT) on YouTube.

Additionally, Nodens originally began as a personal project for my undergraduate Computer Graphics course at UFMG. Early iterations of this framework powered several of my academic projects, including [boids](https://github.com/renanbomtempo/boids) and [galaxians](https://github.com/renanbomtempo/galaxians).

# References

- Gregory, J. (2019) *Game Engine Architecture*. Third edition. CRC Press.
- Horton, I. and Van Weert, P. (2023) Beginning C++23: From Beginner to Pro. Berkeley, CA: Apress. Available at: https://doi.org/10.1007/978-1-4842-9343-0.
- The Cherno (no date) Game Engine Series, YouTube. Available at: http://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT.

# License
Copyright (c) 2026 EldritchCodex.
This project and its entire history are licensed under the MIT License.
