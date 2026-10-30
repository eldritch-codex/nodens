# Building and Toolchain

How to build Nodens, what the build needs, and what to do when it fails. For the shortest path to a running window, see [Getting Started](./getting-started.md).

> [!NOTE]
> **Mental model.** Three things have to line up. Nodens is a C++23 *modules* project, so CMake must drive Ninja. `import std;` needs a recent compiler, and the image provides Clang. The development image already holds both tools and makes them the defaults, so inside it a plain configure just works.

## Prerequisites

| Component | Requirement |
|---|---|
| Operating system | Linux is the supported and validated platform. Native Windows and macOS builds are not supported |
| CMake | 3.30 or newer |
| Compiler | Clang, which the development image provides and makes the default, and which CI uses. GCC 16 also builds Nodens but is untested |
| Build generator | Ninja, because C++ modules require dependency scanning between interfaces, implementation units and importers |
| Doxygen | Optional, only for the `nodens-docs` target, and present in the image |

The development image sets `CC=clang`, `CXX=clang++`, `CMAKE_GENERATOR=Ninja` and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, so a plain `cmake -S . -B build` needs no compiler or generator flags. Outside the image, export the same variables or pass the matching `-D` options.

## Steps

### 1. Configure

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

**Success looks like** a completed configure. The first one may download dependencies, so it needs network access.

### 2. Build

```shell
cmake --build build
```

**Success looks like** the framework and, when Nodens is the top-level project, the example applications building to completion.

### 3. Build one target, when you want less

```shell
cmake --build build --target Nodens
```

**Success looks like** only the framework being rebuilt, without the examples.

### 4. Start over when the metadata is stale

A build directory remembers its compiler, and stale module metadata fails in confusing ways. Remove it and configure again:

```shell
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Options

| Setting | Default | Effect |
|---|---|---|
| `CMAKE_BUILD_TYPE=Debug` | Generator dependent | Defines `ND_DEBUG` and, for Vulkan, enables the validation layers |
| `CMAKE_BUILD_TYPE=Release` | Generator dependent | Defines `ND_RELEASE` |
| `CMAKE_BUILD_TYPE=Profiling` | Generator dependent | Defines `ND_RELEASE` and compiles Tracy in; see [Profiling with Tracy](./profiling-with-tracy.md) |
| `ND_ENABLE_OPENGL` | `ON` | Builds the OpenGL backend |
| `ND_ENABLE_VULKAN` | `ON` | Builds the Vulkan backend when Vulkan is available |
| `ND_BUILD_EXAMPLES` | `ON` when Nodens is top-level, `OFF` when consumed | Adds the example applications |
| `ND_ADD_TRACY_PROFILER_TARGET` | `ON` | Adds the Tracy GUI as the `tracy-profiler` target, excluded from the default build |

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Graphics backends

`ND_ENABLE_OPENGL` and `ND_ENABLE_VULKAN` choose which backends are *compiled*; `FApplicationConfig::GraphicsAPI` chooses which one *runs*, with Vulkan as the default.

```shell
cmake -S . -B build-vulkan -DND_ENABLE_OPENGL=OFF -DND_ENABLE_VULKAN=ON
```

Two consequences follow, and both are startup failures rather than build failures:

- **A missing Vulkan SDK is only a warning.** CMake reports it and builds without the backend, which leaves an OpenGL-only framework.
- **The default application then exits at startup** with `Nodens was built without Vulkan support!`, and an OpenGL application built against a framework without OpenGL exits with `Nodens was built without OpenGL support!`.

The result reaches Nodens' own sources as the compile definitions `ND_HAS_VULKAN` and `ND_HAS_OPENGL`, which are private to the `Nodens` target, so an application does not see them. To check a build, search the compile database, which CI also does:

```shell
grep -c -- '-DND_HAS_VULKAN' build/compile_commands.json
```

A count of zero means the build has no Vulkan backend.

## Troubleshooting

**CMake cannot find a compatible compiler or standard library.** Use the development image first. Building natively means checking that the compiler supports the C++23 module features the project enables and that CMake is at least 3.30.

**The application exits with "Nodens was built without Vulkan support!".** The backend was not built, usually because the SDK was missing at configure time. Install the Vulkan development files or use the image, reconfigure in a fresh directory, and confirm with the `grep` above.

**Module dependency or BMI errors.** Use Ninja, and remove the build directory before reconfiguring, as in step 4.

**A Zed task says `devcontainer: command not found`.** Install the CLI with `npm install -g @devcontainers/cli` and restart Zed, which keeps the `PATH` it started with.

**Configure cannot download dependencies.** The first configure fetches from upstream repositories. Check network access from inside the container and read the CMake error for the dependency that failed.

**The `nodens-docs` target does not exist.** Doxygen was not found, which can only happen outside the image. Install it and reconfigure.

## Reference

### C++ modules and CMake

The top-level build enables the module features:

- `CMAKE_CXX_STANDARD` is 23.
- `CMAKE_CXX_MODULE_STD` is `ON`, so targets can `import std;`. The vendored ImGui, ImPlot and ImPlot3D targets turn it off for themselves, since they are plain C++ libraries.
- Public module interfaces are registered through a `FILE_SET` with `TYPE CXX_MODULES`.
- `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` holds the identifier the project's CMake expects. Do not change it casually; update it only when deliberately moving to a newer toolchain.

A consuming project must set `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` **before** its own `project(... LANGUAGES CXX)` call, because Nodens cannot enable that toolchain feature after C++ is enabled.

### Dependencies

Declarations are centralized in `cmake/NodensDependencies.cmake`:

| Dependency | Revision |
|---|---|
| spdlog | `v1.17.0` |
| GLFW | `3.4` |
| Tracy | `v0.14.1` |
| ImGui | `v1.92.9b-docking` |
| ImPlot | `v1.1` |
| ImPlot3D | `v0.4` |

ImGui, ImPlot and ImPlot3D are built from source by the project's own CMake files. Nodens provides no installable or exported CMake package; consumers use `FetchContent` or `add_subdirectory`, as in [Getting Started](./getting-started.md).

### The development image

Application projects use one published image, `ghcr.io/eldritch-codex/arch-dev:main`. The image owns the full toolchain, and GPU runtime arguments are configured per project in two Dev Container configurations, `nvidia` and `dri`, each with its own container name to avoid collisions.

The `main` tag moves when the image is rebuilt, so its compiler version can change. Pull it again if a local build behaves differently from CI:

```shell
docker pull ghcr.io/eldritch-codex/arch-dev:main
```

The image is maintained in the `eldritch-codex/devcontainers` repository, in its `arch-dev` directory, where its package list and pinned Slang installation live. On another distribution, install equivalent packages with that distribution's manager. To change the image itself, build it from that directory and reopen the container:

```shell
docker build --pull -t ghcr.io/eldritch-codex/arch-dev:main arch-dev
```

#### NVIDIA hosts

Three host-side pieces are needed: a working NVIDIA driver, Docker or another supported engine, and the NVIDIA Container Toolkit configured for that engine. The image carries no kernel driver.

```shell
nvidia-smi
sudo pacman -S nvidia-container-toolkit
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
docker run --rm --gpus=all nvidia/cuda:12.9.0-base-ubuntu22.04 nvidia-smi
```

The last command is the test that matters: if it prints the GPU table, the container runtime can reach the driver. Intel and AMD hosts need none of this; open the `dri` configuration instead, which passes the render device and the video and render groups.

### The editor outside the container

Zed can stay on the host while its tasks and debugger run inside the container, through the Dev Containers CLI. The repository, the template and Nyar track their `.zed` tasks, debug scenarios and settings for this, and there is no script behind them: each task is a `devcontainer exec` command you can read.

```shell
npm install -g @devcontainers/cli
```

Run the **Docker: Start** task once per session; it runs `devcontainer up`, which creates or starts the container from the project's `devcontainer.json`. The other tasks then use `devcontainer exec` with the same file:

```shell
devcontainer exec --workspace-folder . \
    --config .devcontainer/nvidia/devcontainer.json \
    cmake --build build -j4
```

Both configurations mount the project at its host path, so the build directory, the compile database and debugger source paths are the same inside and outside. The tracked tasks use `nvidia`; on Intel or AMD, replace it with `dri` in the `.zed` files.

The debugger runs the CodeLLDB adapter inside the container, reached by `devcontainer exec` and spoken to over stdio, so no port is published and several projects can be debugged at once. The image ships the adapter at `/opt/codelldb/adapter/codelldb`.

### A native Linux host

The container is recommended because it defines the tested environment. A native build uses the same CMake commands after installing equivalent packages and exporting the defaults:

```shell
export CC=clang CXX=clang++
export CMAKE_GENERATOR=Ninja
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Continuous integration

GitHub Actions runs on every pull request into `main`, on every push to `main`, and on `v*` tags, using the same image, so CI and local builds share a toolchain:

- **Format.** `clang-format` from the image, checked only on the C++ files a change touches.
- **Docs.** Every relative Markdown link under `docs/` resolves to a path in the tree.
- **Build.** `Debug`, `Release` and `Profiling` are each configured with Clang, checked for `ND_HAS_VULKAN`, and built.
- **Release.** A `v*` tag builds and publishes a GitHub Release with a source archive and the changelog section for that version.

The workflow is `.github/workflows/ci.yml`. There is no test suite yet, so CI proves that the framework builds in three configurations and is formatted, not that it behaves correctly. [Branching and releases](../BRANCHING.md) covers the branch and release rules.

### API documentation

```shell
cmake --build build --target nodens-docs
```

The output is written inside the build tree, at the location `Doxyfile.in` sets.

## See Also

- [Getting Started](./getting-started.md): the template and the first run.
- [Architecture Overview](./architecture-overview.md): what the build produces.
- [Profiling with Tracy](./profiling-with-tracy.md): the `Profiling` configuration in detail.
