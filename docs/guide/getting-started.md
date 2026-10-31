# Getting Started

Go from nothing to a running Nodens window by starting from the application template.

> [!NOTE]
> **Mental model.** Your application is two pieces of code that CMake joins together: yours, and Nodens. The template is a small CMake project that downloads Nodens at a fixed release tag, compiles everything with Clang inside the published devcontainer image, and links your program against `Nodens::Nodens`.

> ⚠️ **Linux only.** The published development image builds and validates Nodens applications on Linux. Native Windows and macOS builds are not supported. See [Building and Toolchain](./building-and-toolchain.md).

## Prerequisites

| You need | Why |
|---|---|
| Docker | The toolchain, desktop libraries and GPU runtime live in the published image |
| An editor with Dev Container support | [Zed](https://zed.dev/), or [Visual Studio Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) |
| The Dev Containers CLI, only if the editor stays on the host | The tracked Zed tasks and debug scenarios call `devcontainer`; install it with `npm install -g @devcontainers/cli` |
| The NVIDIA Container Toolkit, only on an NVIDIA host | The `nvidia` configuration requests the GPU; the `dri` configuration needs nothing extra on Intel and AMD |

Everything else arrives in one image, `ghcr.io/eldritch-codex/arch-dev:main`: the C++ toolchain, the X11 and Wayland development files that GLFW builds against, OpenGL and Vulkan tools, and the shader tooling. The template and the Nodens repository each ship two Dev Container configurations, `nvidia` and `dri`; your editor asks which to open.

## Steps

### 1. Create your project

On the [Nodens Application Template](https://github.com/eldritch-codex/template-nodens-app), select **Use this template** to create your repository.

**Success looks like** a new repository of your own, holding a `CMakeLists.txt` and a `src` directory with the application's entry point.

### 2. Open it in the container

Open the repository in your editor and reopen it in the dev container, choosing `nvidia` or `dri`.

**Success looks like** an editor whose terminal reports the image's tools:

```shell
clang++ --version
cmake --version
```

Both must print a version. If `cmake` reports older than 3.30, the editor opened the project on the host rather than in the container.

### 3. Configure and build

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target NodensApp
```

The container's default compiler is Clang, the one Nodens is built and tested with, so no compiler flag is needed.

**Success looks like** a linked executable named `NodensApp`, inside the build directory.

### 4. Run it

```shell
./build/NodensApp
```

**Success looks like** a window opening, empty except for the ImGui overlay's dockspace. That is a complete Nodens application.

### 5. Read the two files that matter

The CMake file that fetches Nodens:

```cmake
cmake_minimum_required(VERSION 3.30)

# CMake must know about experimental standard-library module support before
# project() enables the C++ language. Nodens requires this when consumed via
# FetchContent or add_subdirectory().
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(NodensApp VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(nodens
    GIT_REPOSITORY https://github.com/eldritch-codex/nodens.git
    GIT_TAG        v0.3.0 # A release tag; change it to move to a newer Nodens release
)
FetchContent_MakeAvailable(nodens)

add_executable(NodensApp src/main.cpp)
target_link_libraries(NodensApp PRIVATE Nodens::Nodens)
```

The `GIT_TAG` pins one Nodens release, so every build gets the same framework version.

The application class, which subclasses `Nodens::Application` and describes itself with an `FApplicationConfig`:

```cpp
import nodens;

class MyApp : public Nodens::Application {
public:
  static inline const Nodens::FApplicationConfig appConfig = {
      .Name = "My Nodens Application",
      .WindowWidth = 1280,
      .WindowHeight = 720,
      .Presentation = Nodens::EPresentation::WindowImGUI,
      .ShouldImGuiBlockInputs = true,
      .Theme = Nodens::ETheme::Dark};

  MyApp() : Application(appConfig) {
    // Add layers here, e.g., PushLayer(new MyLayer());
  }
};

int main() {
  Nodens::InitializeLoggers();
  auto app = MyApp();
  app.Run();

  return 0;
}
```

`FApplicationConfig` also carries `VSync`, `GraphicsAPI` (Vulkan by default) and `Pacing`. To put your own code in the application, continue with [Creating a Custom Layer](./creating-a-custom-layer.md).

### 6. Move to a newer Nodens

Change the tag in `FetchContent_Declare` to a release from the [Nodens releases](https://github.com/eldritch-codex/nodens/releases), then reconfigure and build again.

**Success looks like** a clean build against the new tag. While Nodens is at 0.x, a release that changes the API bumps the minor version, so read the [changelog](../../CHANGELOG.md) before moving to a newer minor version.

### 7. Develop the framework and your application together

Clone the Nodens repository only when you want to change the framework itself, then replace `FetchContent` with a local checkout:

```cmake
add_subdirectory(path/to/Nodens)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE Nodens::Nodens)
```

**Success looks like** your application building against the checkout, with your change visible in its behaviour.

`CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` must still come before `project()` in your own CMake file, because Nodens cannot enable that toolchain feature after your project has enabled C++. This still builds Nodens from source; an installable CMake package is not provided.

## Troubleshooting

**The build stops at `cmake --version` reporting an old version.** The editor opened the folder on the host. Reopen it in the container, which carries CMake 3.30 or newer.

**The application exits with "Nodens was built without Vulkan support!".** The framework was configured without the Vulkan backend, usually because the SDK was missing when CMake ran. Use the container, with a fresh build directory. [Building and Toolchain](./building-and-toolchain.md) has the check.

**A Zed task prints `devcontainer: command not found`.** Install the Dev Containers CLI and restart Zed, which keeps the `PATH` it started with.

**Configure fails while downloading.** The first configuration fetches dependencies from their upstream repositories, so it needs network access from inside the container.

## See Also

- [Creating a Custom Layer](./creating-a-custom-layer.md): add your own behaviour.
- [Architecture Overview](./architecture-overview.md): how the framework is structured.
- [Building and Toolchain](./building-and-toolchain.md): compilers, options, dependencies and the rest of the troubleshooting.
