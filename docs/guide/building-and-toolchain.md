# Building and Toolchain

Everything about building Nodens: the toolchain it needs, the options it has and what to do when a build fails. For the shortest path to a running window, see [Getting Started](./getting-started.md).

> [!NOTE]
> **Mental model.** Three things have to line up. Nodens is a C++23 *modules* project, so CMake must drive Ninja. `import std;` needs a recent compiler: the image provides Clang. The devcontainer image already holds both tools and makes Clang and Ninja the defaults, so inside it a plain `cmake` configure just works.
>
> The page runs from doing to understanding:
>
> 1. **Build it.** [Configure and build](#configuring-and-building-nodens)
> 2. **Know what it needs.** [Requirements](#requirements)
> 3. **Turn things on and off.** [Build types and options](#build-types-and-options) and [graphics backends](#graphics-backends)
> 4. **Understand the machinery.** [Modules](#c-modules-and-cmake) and [dependencies](#dependency-resolution)
> 5. **Set up the environment.** [Development image and GPUs](#the-development-image)
> 6. **Check it in CI, or fix it.** [CI](#continuous-integration) and [troubleshooting](#troubleshooting)

## Configuring and Building Nodens

From the Nodens repository root, preferably in the active devcontainer:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

When Nodens is the top-level project, the example applications are enabled by default. To build only the framework target, use:

```shell
cmake --build build --target Nodens
```

The first configuration may download dependencies and therefore requires network access. If the build directory contains stale CMake or module metadata, remove it and configure again:

```shell
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Requirements

Linux is the supported and validated platform. The repository's devcontainer uses Arch Linux and is the recommended way to get the toolchain; native Linux builds work with equivalent packages. Native Windows and macOS builds are not supported or validated.

The project currently requires:

| Component | Requirement or guidance |
|-----------|--------------------------|
| Operating system | Linux for the supported and validated workflow |
| CMake | 3.30 or newer, as required by the top-level CMakeLists.txt |
| Language | C++23 with standard library module support (`import std;`) |
| Compiler | **Clang.** The development image provides it and makes it the default, and CI builds with it. GCC 16 also builds Nodens but is not tested |
| Build generator | Ninja, for C++ module dependency scanning |
| Doxygen | Optional; only for the `nodens-docs` target (in the image) |

### Defaults in the image

The development image sets the CMake defaults Nodens needs: `CC=clang`, `CXX=clang++`, `CMAKE_GENERATOR=Ninja` and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`. A plain `cmake -S . -B build` therefore needs no compiler or generator flags. Outside the image, export the same variables, or pass the matching `-D` options. A build directory remembers its compiler, so after changing it, delete the directory and configure again.

### The shared image

The shared Linux graphics development image is maintained in the [`eldritch-codex/devcontainers`](https://github.com/eldritch-codex/devcontainers) repository, in its `arch-dev` directory. Its Arch package list is in [`arch-dev/scripts/install-packages.sh`](https://github.com/eldritch-codex/devcontainers/blob/main/arch-dev/scripts/install-packages.sh), and the pinned Slang compiler installation is in [`arch-dev/scripts/install-slang.sh`](https://github.com/eldritch-codex/devcontainers/blob/main/arch-dev/scripts/install-slang.sh). On another Linux distribution, install equivalent packages using that distribution's package manager.

## Build Types and Options

The project defines the following build types and options:

| Setting | Default | Description |
|---------|---------|-------------|
| `CMAKE_BUILD_TYPE=Debug` | Depends on the generator | Enables the `ND_DEBUG` compile definition and, for Vulkan, the validation layers. |
| `CMAKE_BUILD_TYPE=Release` | Depends on the generator | Enables the `ND_RELEASE` compile definition for Release builds. |
| `CMAKE_BUILD_TYPE=Profiling` | Depends on the generator | Enables `ND_RELEASE` and Tracy instrumentation. See [Profiling with Tracy](./profiling-with-tracy.md). |
| `ND_ENABLE_OPENGL` | `ON` | Builds the OpenGL backend. |
| `ND_ENABLE_VULKAN` | `ON` | Builds the Vulkan backend when Vulkan is available. |
| `ND_BUILD_EXAMPLES` | `ON` when Nodens is top-level; `OFF` when consumed through `FetchContent` or `add_subdirectory` | Controls whether the example applications are added. |
| `ND_ADD_TRACY_PROFILER_TARGET` | `ON` | Adds the Tracy Profiler GUI from the fetched Tracy source as the `tracy-profiler` target. The target is excluded from the default build, so it is built only when requested. |

For a Release build:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Graphics backends

`ND_ENABLE_OPENGL` and `ND_ENABLE_VULKAN` choose which backends are *compiled*; the application's `GraphicsAPI` setting chooses which one *runs*. The default `GraphicsAPI` is Vulkan.

A Vulkan-only configuration:

```shell
cmake -S . -B build-vulkan \
    -DND_ENABLE_OPENGL=OFF -DND_ENABLE_VULKAN=ON
```

and an OpenGL-only one swaps the two options. Two things to know:

- **A missing Vulkan SDK is a configure warning.** If CMake cannot find Vulkan it prints a warning and builds without the backend, giving an OpenGL-only build.
- **The default then fails at startup.** An application whose `GraphicsAPI` is the default Vulkan, built into a framework without Vulkan, exits at startup with `Nodens was built without Vulkan support!`. Likewise a framework without OpenGL exits with `Nodens was built without OpenGL support!` if you select OpenGL.

Nodens' own sources see the result as the compile definitions `ND_HAS_VULKAN` and `ND_HAS_OPENGL`, which are defined only when the backend was actually built. They are private to the `Nodens` target, so your application does not see them. To check a build, search the compile database: `grep -c -- '-DND_HAS_VULKAN' build/compile_commands.json` is zero for a build without Vulkan. CI makes exactly this check.

### Profiling build

`Profiling` mirrors Release and adds what the profiler needs; Tracy is compiled in for it only. The commands, including building the profiler GUI, are in [Profiling with Tracy](./profiling-with-tracy.md).

## C++ Modules and CMake

Nodens uses C++ module interfaces (`.cppm`) and implementation units (`.cpp`). The top-level build enables CMake's module-related features:

- `CMAKE_CXX_STANDARD` is set to 23.
- `CMAKE_CXX_MODULE_STD` is `ON`, so targets can `import std;`. The vendored ImGui, ImPlot and ImPlot3D targets turn it off for themselves, since they are plain C++ libraries.
- Public module interfaces are registered through a `FILE_SET` with `TYPE CXX_MODULES`.
- `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` is set to the identifier expected by the project's CMake configuration.

The experimental `import std;` support depends on compatible CMake, compiler, and standard-library versions. Do not change the experimental identifier casually; update it only when deliberately updating the supported CMake toolchain.

When Nodens is consumed through `FetchContent` or `add_subdirectory`, the consuming project's CMakeLists.txt must set `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` **before** its `project(... LANGUAGES CXX)` call. Nodens cannot enable this toolchain feature after the parent project has already enabled C++.

Ninja is the recommended generator because C++ modules require the build system to understand compilation dependencies between module interfaces, implementation units, and importers.

## Dependency Resolution

Dependency declarations are centralized in [`cmake/NodensDependencies.cmake`](../../cmake/NodensDependencies.cmake). The current source revisions are:

| Dependency | Revision |
|------------|----------|
| spdlog | `v1.17.0` |
| GLFW | `3.4` |
| Tracy | `v0.14.1` |
| ImGui | `v1.92.9b-docking` |
| ImPlot | `v1.1` |
| ImPlot3D | `v0.4` |

CMake may download and build these dependencies during configuration. ImGui, ImPlot, and ImPlot3D are built from source by the project's CMake files because they do not provide the packaging model used by Nodens.

Nodens does not currently provide an installable or exported CMake package. Consumers should use `FetchContent` or a local `add_subdirectory` checkout, as described in [Getting Started](./getting-started.md).

## The development image

Application projects use one published image:

```text
ghcr.io/eldritch-codex/arch-dev:main
```

The image owns the full toolchain. GPU runtime arguments and NVIDIA capabilities
are configured per project, in two Dev Container configurations under
`.devcontainer/`: `nvidia` and `dri` (Intel and AMD). Your editor asks which one
to open. Each config uses a unique Docker container name to avoid local name
collisions.

The image tag `main` moves when the image is rebuilt, so the compiler version inside it can change. Pull it again (`docker pull ghcr.io/eldritch-codex/arch-dev:main`) if your local copy is old and a build behaves differently from CI.

### NVIDIA GPUs

NVIDIA GPU support requires three host-side components:

1. A working NVIDIA driver.
2. Docker or another supported container engine.
3. NVIDIA Container Toolkit configured for that container engine.

The development image does not contain NVIDIA kernel drivers. Install the
NVIDIA driver on the host machine first, then verify it:

```shell
nvidia-smi
```

The [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
connects Docker to the host NVIDIA driver. When a container requests
`--gpus=all`, the toolkit injects NVIDIA GPU devices and compatible userspace
driver libraries into the container. Nodens also sets
`NVIDIA_DRIVER_CAPABILITIES=graphics,display,utility` to request graphics,
display, and utility capabilities. This setting does not install a driver.

On Arch Linux, install the toolkit from the official `Extra` repository:

```shell
sudo pacman -Syu
sudo pacman -S nvidia-container-toolkit
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
```

Install the host NVIDIA driver separately with the package appropriate for the
GPU and kernel. On Ubuntu, Debian, Fedora, and other supported distributions,
follow the [NVIDIA Container Toolkit installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
for distribution-specific repository and package instructions.

Test Docker GPU access before opening the Dev Container:

```shell
docker run --rm --gpus=all nvidia/cuda:12.9.0-base-ubuntu22.04 nvidia-smi
```

Intel/AMD hosts do not need NVIDIA Container Toolkit. Open the `dri`
configuration, which passes `/dev/dri` and the `video` and `render` groups.

#### Rebuilding the image locally

To test changes to the image, clone the
[Devcontainers repository](https://github.com/eldritch-codex/devcontainers) and
build the image from its `arch-dev` directory, which is the build context:

```shell
docker build --pull \
    -t ghcr.io/eldritch-codex/arch-dev:main \
    arch-dev
```

Rebuild or reopen the project Dev Container after changing the image.

### Zed outside the container

The editor does not have to run inside the container. Zed can stay on the host while its tasks and debugger run in the container, through the [Dev Containers CLI](https://github.com/devcontainers/cli). The Nodens repository, the template and Nyar track their `.zed/tasks.json`, `.zed/debug.json` and `.zed/settings.json` for this. There is no script behind them: each task is a `devcontainer exec` command you can read.

Install the CLI once, and restart Zed so it sees the new command:

```shell
npm install -g @devcontainers/cli
```

Then run the **Docker: Start** task once per session. It runs `devcontainer up`, which creates the container from the project's `devcontainer.json` or starts it again. The Config, Build and Run tasks use `devcontainer exec` with the same file:

```shell
devcontainer exec --workspace-folder . \
    --config .devcontainer/nvidia/devcontainer.json \
    cmake --build build -j4
```

Both GPU configurations mount the project at its host path (`workspaceMount` and `workspaceFolder`), so the `build` directory, `compile_commands.json` and debugger source paths are the same inside and outside the container. The tasks use the `nvidia` configuration. On Intel or AMD, replace `nvidia` with `dri` in the three `.zed` files.

The debugger uses the CodeLLDB adapter inside the container. `.zed/settings.json` points Zed's `CodeLLDB` adapter at `devcontainer exec … /opt/codelldb/adapter/codelldb`, and Zed talks to it over stdio, so no port is published and several projects can be debugged at once. The development image ships the adapter, pinned and checksum-verified, at `/opt/codelldb/adapter/codelldb`.

## Building on a Native Linux Host

The devcontainer is recommended because it defines the tested environment. A native Linux build can use the same CMake commands after installing equivalent packages.

The shared image's Arch Linux package list is maintained in the [Devcontainers repository](https://github.com/eldritch-codex/devcontainers/blob/main/arch-dev/scripts/install-packages.sh). The script uses `pacman` and is intended to run inside the Arch-based image; it is a reference for package names rather than a universal installer.

After installing the equivalent packages, configure and build from the Nodens repository root, after exporting the defaults above (`CC=clang CXX=clang++ CMAKE_GENERATOR=Ninja`):

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Continuous Integration

GitHub Actions runs on every pull request into `main`, on every push to `main`, and on `v*` tags. It uses the same development image as the devcontainer, so CI and local builds share a toolchain:

- **Format.** `clang-format` from the image, checked only on the C++ files a change touches.
- **Docs.** Every relative Markdown link under `docs/` resolves to a path in the tree.
- **Build.** `Debug`, `Release` and `Profiling` are each configured with Clang, checked for `ND_HAS_VULKAN`, and built.
- **Release.** A `v*` tag builds the tag and publishes a GitHub Release with a source archive and the changelog section for that version as the notes.

The workflow is [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml), and the branching and release model is in [Branching and releases](../BRANCHING.md). There is no test suite yet, so CI proves that the framework builds in all three configurations and is formatted, not that it behaves correctly.

### Checking a change

Every change reaches `main` through a short-lived branch and a squashed pull request, so the format, docs and build jobs above run on it before it lands. Nothing is committed straight to `main`. The branch names, the merge step and the release procedure are in [Branching and releases](../BRANCHING.md).

## Generating API Documentation

The development image includes Doxygen, so the `nodens-docs` target exists:

```shell
cmake --build build --target nodens-docs
```

The output is written inside the build tree, at the location `Doxyfile.in` sets.

## Troubleshooting

### CMake cannot find a compatible compiler or standard library

Use the devcontainer first. If building natively, verify that your compiler supports the C++23 module features enabled by the project and that CMake is at least version 3.30.

### The application exits with "Nodens was built without Vulkan support!"

The framework was built without the Vulkan backend, usually because the Vulkan SDK was not found at configure time. Install the Vulkan development files, or use the devcontainer, reconfigure in a fresh build directory, and confirm with `grep -c -- '-DND_HAS_VULKAN' build/compile_commands.json`. See [Graphics backends](#graphics-backends).

### Module dependency or BMI errors

Use Ninja and remove the build directory before reconfiguring:

```shell
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### A Zed task says `devcontainer: command not found`

The Zed tasks and the debugger run the `devcontainer` command from the Dev Containers CLI. Install it with `npm install -g @devcontainers/cli` and restart Zed, which keeps the `PATH` it started with. See [Zed outside the container](#zed-outside-the-container).

### Configuration cannot download dependencies

The first configuration requires network access because dependencies may be fetched from their upstream repositories. Check network access from inside the devcontainer and inspect the CMake error for the dependency that failed to populate.

### The documentation target does not exist

Doxygen is missing, which can only happen outside the development image. Install it and reconfigure; the target is defined only when CMake finds Doxygen.

## See Also

- [Getting Started](./getting-started.md): Onboarding, devcontainer setup, and application integration.
- [Architecture Overview](./architecture-overview.md): Framework structure and subsystem responsibilities.
- [Profiling with Tracy](./profiling-with-tracy.md): The `Profiling` configuration in detail.
