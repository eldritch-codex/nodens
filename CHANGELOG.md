# Changelog

All notable changes to Nodens are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `scripts/check-docs-links.sh` fails on a relative Markdown link under `docs/` that points at a path missing from the tree. The CI `docs` job runs it.
- `docs/STYLE.md` states the writing standard for every page: three page shapes, a glossary every page shares, the voice each folder uses, and a checklist to apply before a pull request.
- `FPacingConfig::Pacing` selects a fixed-step mode (`EPacing::Fixed`) next to the default variable one: the pacer accumulates wall time into steps of `FixedDelta`, caps catch-up at `MaxStepsPerFrame`, and sleeps to the step boundary so the loop does not spin. `examples/headlesspulse --fixed` runs it, and the example ends each run with a summary of steps, simulated time, wall time and time per step, so the two modes compare run by run.
- `TaskSystem::Submit` moves the callable and its arguments into the task, which is built from a lambda and `std::invoke` instead of `std::bind` and a `std::shared_ptr<std::packaged_task>`. A move-only argument now reaches a by-value parameter, `std::bind`'s decayed copies are gone, and the shared_ptr node, with its copy taken when the task was queued, is no longer allocated. `Submit` is also constrained on the decayed callable and argument types, which is what the task invokes.

### Changed

- `FApplicationConfig` states its window and overlay choice as one `EPresentation` value (`Window`, `WindowImGUI`, `Headless`) instead of the `EnableGUI` and `CreateWindow` booleans, which could express a GUI with no window. `HasWindow()` reads that value, and a config that set either boolean moves to the new field.
- `Application::Run()` takes each frame's time from the new `Nodens.FramePacer` module instead of reading the window, and clamps it to `FPacingConfig::MaxFrameDelta` (250 ms by default), so a dragged window or a suspended process no longer reaches layers as one long step. `FApplicationConfig` gains `Pacing` for the policy, and the old `m_LastFrameTime` member is gone.
- Every change goes through a short-lived branch and a squashed pull request, whatever its type. The direct-to-`main` route for `docs`, `style`, `chore` and tooling `build` commits is removed. See `docs/BRANCHING.md`.
- The tutorial and architecture pages moved from the GitHub wiki into `docs/guide/`, one page per file, with an index at `docs/guide/README.md` and every link written as a repo-relative path. The pages ship with the code, so a change to an API and its explanation land in one commit.
- `docs/` is split by audience: `guide/` for application writers, `dev/` for the design notes on backends, C++ modules, dependencies and type names, and the branching and committing pages at the root, with `docs/README.md` as the index of all three.
- The CMake option `ND_BUILD_TRACY_PROFILER` is renamed `ND_ADD_TRACY_PROFILER_TARGET`, since it adds the `tracy-profiler` target rather than building it: the target stays out of the default build and is built by name. The default is now `ON`, which only declares the target; a configuration that passed the old option must pass the new one.

### Removed

- `scripts/check.sh`. Every change now gets a CI run on its pull request before it lands, so the local copy of the CI steps is gone.

## [0.2.1] - 2026-10-27

### Added

- `scripts/check.sh` runs the format check on the changed C++ files and a Debug build in `build/check`, in the dev container. It is the check to run before a direct commit that touches C++ or the build.

### Changed

- Small changes (`docs`, `style`, `chore`, tooling `build`) are committed directly to `main`, and risky ones (`feat`, `fix`, `refactor`, `perf`, `ci`, and `build` that changes `CMakeLists.txt` or a dependency version) go through a squashed pull request. A failing `main` is fixed before anything else, and a tag is created only on a commit whose CI run succeeded. See `docs/BRANCHING.md`.
- The Zed tasks and debug scenarios run in the dev container through the Dev Containers CLI (`@devcontainers/cli`), and `.zed/docker.sh` is gone. The CLI reads the same `devcontainer.json` as the editor, so the container is not described twice. Both GPU configurations mount the project at its host path, so build outputs and debugger source paths match the host.

## [0.2.0] - 2026-10-27

### Changed

- The job system type is `TaskSystem`, renamed from `JobSystem`, and the accessor is `Application::GetTaskSystem()`. In the engine literature a job system drains a queue of jobs that run to completion, with coroutines or fibers for jobs that sleep and counters to join a batch (Gregory 2019, pp. 553-555), while this pool takes one callable and returns a future; see `docs/naming.md`.
- The graphics types are `IGraphicsBackend`, `OpenGLBackend` and `VulkanBackend`, renamed from `IGraphicsContext`, `OpenGLContext` and `VulkanContext`, and `IWindow::GetBackend()` replaces `GetGraphicsContext()`. The interface covers backend initialization and frame presentation, and Vulkan owns an instance, a device and a swapchain rather than a context, so the shared word cannot be context; the window's member is `m_Backend`.
- The theme enum is `ETheme`, renamed from `EDefaultTheme`. Its first member is `None`, so the word default belongs to the appliers, which keep their names, and the theme field on `FApplicationConfig` is `Theme`; the module keeps the name `Nodens.DefaultTheme`.
- The startup structs are `FWindowConfig` and `FApplicationConfig`, renamed from `FWindowProps` and `FApplicationSpecification`, and the names that hold them follow: the window factory and the `Application` constructor take `config`, and the examples hold `appConfig`. `FWindowConfig::API` is now `GraphicsAPI`, matching the enum and the application config.

## [0.1.1] - 2026-10-27

### Changed

- ImPlot is fetched at `v1.1`, up from `v1.0`.
- The dev container has one configuration per GPU type, `.devcontainer/nvidia` and `.devcontainer/dri`, instead of a single file edited by hand. `docker.sh` picks one, and `NODENS_GPU` overrides it.
- The shared Zed tasks and debug scenarios are tracked in the repository, and they run from the project root even when the Zed workspace is a parent folder.

### Fixed

- Configuring without the Vulkan SDK prints a warning. Before, the build was silently OpenGL-only and exited at startup under the default Vulkan API.
- The `circularwave3d` example advances by the frame time, so the curve no longer scrolls faster at a higher frame rate.
- GLFW is fetched at its pinned tag. It was declared but never made available, so the build linked the system copy.
- `ND_BUILD_EXAMPLES` defaults to off when Nodens is consumed by another project. The default was on, because the second `option()` call was a no-op.
- The `MouseButtonReleased` event has a name. It was empty, so the event logged as an empty string and the file failed to compile with GCC.
- `EventBus::Subscribe` takes the registry lock, and `QueueEvent` no longer reads the subscriber list unlocked for its trace line, so subscribing or queueing a new event type from another thread no longer races on that list.

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
