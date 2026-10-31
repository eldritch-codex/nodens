# Changelog

All notable changes to Nodens are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.3.0] - 2026-10-31

### Added

- Fixed-step pacing: set `FPacingConfig::Pacing` to `EPacing::Fixed` to advance the loop in steps of `FixedDelta`. `examples/headlesspulse --fixed` shows it. ([#50](https://github.com/eldritch-codex/nodens/pull/50))
- `scripts/check-docs-links.sh` fails CI on a broken relative link under `docs/`. ([#49](https://github.com/eldritch-codex/nodens/pull/49))
- `docs/STYLE.md`, the writing standard for the guide. ([#51](https://github.com/eldritch-codex/nodens/pull/51))

### Changed

- **Breaking:** `FApplicationConfig` takes one `EPresentation` value (`Window`, `WindowImGUI`, `Headless`) in place of the `EnableGUI` and `CreateWindow` booleans. ([#50](https://github.com/eldritch-codex/nodens/pull/50))
- **Breaking:** the CMake option `ND_BUILD_TRACY_PROFILER` is now `ND_ADD_TRACY_PROFILER_TARGET`, and it defaults to `ON`. ([a9f155d](https://github.com/eldritch-codex/nodens/commit/a9f155d))
- `Application::Run()` takes frame time from the new `Nodens.FramePacer` module and clamps it to `FPacingConfig::MaxFrameDelta` (250 ms by default). ([#50](https://github.com/eldritch-codex/nodens/pull/50))
- `TaskSystem::Submit` accepts move-only arguments and no longer allocates a `std::shared_ptr`. ([15403cc](https://github.com/eldritch-codex/nodens/commit/15403cc))
- The tutorial and architecture pages moved from the wiki into `docs/guide/`, and `docs/` is split into `guide/`, `dev/` and the branching and committing pages. ([#49](https://github.com/eldritch-codex/nodens/pull/49))
- The guide follows the new style standard. ([#51](https://github.com/eldritch-codex/nodens/pull/51))
- Every change goes through a pull request, including `docs`, `style`, `chore` and tooling `build`. See `docs/BRANCHING.md`. ([#49](https://github.com/eldritch-codex/nodens/pull/49))

### Removed

- `scripts/check.sh`, since CI now runs on every pull request. ([#49](https://github.com/eldritch-codex/nodens/pull/49))

## [0.2.1] - 2026-10-27

### Added

- `scripts/check.sh` runs the format check and a Debug build in the dev container. ([dc42eba](https://github.com/eldritch-codex/nodens/commit/dc42eba))

### Changed

- Small changes were committed directly to `main` and risky ones went through a pull request. See `docs/BRANCHING.md`. ([99cd378](https://github.com/eldritch-codex/nodens/commit/99cd378))
- The Zed tasks and debug scenarios run in the dev container through the Dev Containers CLI, and `.zed/docker.sh` is gone. ([240a39d](https://github.com/eldritch-codex/nodens/commit/240a39d))

## [0.2.0] - 2026-10-27

### Changed

- **Breaking:** `JobSystem` is now `TaskSystem`, and the accessor is `Application::GetTaskSystem()`. ([1b2f856](https://github.com/eldritch-codex/nodens/commit/1b2f856))
- **Breaking:** `IGraphicsContext`, `OpenGLContext` and `VulkanContext` are now `IGraphicsBackend`, `OpenGLBackend` and `VulkanBackend`, and `IWindow::GetBackend()` replaces `GetGraphicsContext()`. ([e04f193](https://github.com/eldritch-codex/nodens/commit/e04f193))
- **Breaking:** `EDefaultTheme` is now `ETheme`, and the theme field on `FApplicationConfig` is `Theme`. ([96df9d3](https://github.com/eldritch-codex/nodens/commit/96df9d3))
- **Breaking:** `FWindowProps` and `FApplicationSpecification` are now `FWindowConfig` and `FApplicationConfig`, and `FWindowConfig::API` is now `GraphicsAPI`. ([e1d8622](https://github.com/eldritch-codex/nodens/commit/e1d8622))

## [0.1.1] - 2026-10-27

### Changed

- ImPlot is fetched at `v1.1`, up from `v1.0`. ([9f61e54](https://github.com/eldritch-codex/nodens/commit/9f61e54))
- The dev container has one configuration per GPU type, `.devcontainer/nvidia` and `.devcontainer/dri`. ([#39](https://github.com/eldritch-codex/nodens/pull/39))
- The shared Zed tasks and debug scenarios are tracked in the repository. ([#39](https://github.com/eldritch-codex/nodens/pull/39))

### Fixed

- Configuring without the Vulkan SDK prints a warning instead of building silently OpenGL-only. ([#40](https://github.com/eldritch-codex/nodens/pull/40))
- `circularwave3d` advances by the frame time, so it no longer runs faster at a higher frame rate. ([#41](https://github.com/eldritch-codex/nodens/pull/41))
- GLFW is fetched at its pinned tag instead of linking the system copy. ([#45](https://github.com/eldritch-codex/nodens/pull/45))
- `ND_BUILD_EXAMPLES` defaults to off when Nodens is consumed by another project. ([#45](https://github.com/eldritch-codex/nodens/pull/45))
- `MouseButtonReleased` has a name, which fixes an empty log string and a GCC compile error. ([#46](https://github.com/eldritch-codex/nodens/pull/46))
- `EventBus` no longer races on the subscriber list when another thread subscribes or queues an event. ([#46](https://github.com/eldritch-codex/nodens/pull/46))

## [0.1.0] - 2026-10-24

### Added

- Application lifecycle built around a layer stack, with an application specification that selects the graphics API.
- GLFW windowing with OpenGL and Vulkan backends, each gated by a CMake option.
- Event system: typed events routed through the layer stack, plus a thread-safe publish and subscribe bus for queued dispatch.
- Input polling with platform-independent key and mouse-button codes.
- Job system built on C++20 jthreads.
- Dear ImGui integration with ImPlot and ImPlot3D, a renderer abstraction per backend, and a default theme.
- Tracy CPU and GPU profiling behind a dedicated `Profiling` build configuration.
- Example applications: `circularwave3d` and `planetaryscan`.
- GitHub Actions CI: Debug, Release and Profiling builds with clang in the development image, a Vulkan backend assertion, formatting checks on changed files, and a tag-triggered release.
- Branching and release policy in `docs/BRANCHING.md`.

### Changed

- OpenGL and Vulkan backends are optional at configure time.
- The Vulkan context, command buffers and frame synchronization are owned by Nodens and borrowed by renderers.
- The default graphics API is Vulkan.
