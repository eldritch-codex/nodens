# Changelog

All notable changes to Nodens are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-10-24

### Added

- Application lifecycle built around a layer stack, with an application specification that selects the graphics API.
- GLFW windowing with OpenGL and Vulkan backends, each gated by a CMake option.
- Event system: typed events routed through the layer stack, plus a thread-safe publish and subscribe bus for queued dispatch.
- Input polling with platform-independent key and mouse-button codes.
- Job system built on C++20 jthreads [^1].
- Dear ImGui integration with ImPlot and ImPlot3D, a renderer abstraction per backend, and a default theme.
- Tracy CPU and GPU profiling behind a dedicated `Profiling` build configuration.
- Example applications: `circularwave3d` and `planetaryscan`.
- GitHub Actions CI: Debug and Release builds in the development image, a Vulkan backend assertion, formatting checks on changed files, and a tag-triggered release.
- Branching and release policy in `docs/BRANCHING.md`.

### Changed

- OpenGL and Vulkan backends are optional at configure time.
- The Vulkan context, command buffers and frame synchronization are owned by Nodens and borrowed by renderers.
- The default graphics API is Vulkan.

[^1]: `src/Nodens/nodens.cppm:14` describes the concurrency module as a multithreaded job system built on C++20 jthreads.
