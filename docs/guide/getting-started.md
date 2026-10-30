# Getting Started

Go from nothing to a running Nodens window by starting from the application template.

> [!NOTE]
> **Mental model.** Your application is two pieces of code that CMake joins together: yours, and Nodens. The template is a small CMake project that downloads Nodens at a fixed release tag, compiles everything with Clang inside the published devcontainer image, and links your program against `Nodens::Nodens`.
>
> The steps, in order:
>
> 1. **Install the tools.** [Prerequisites](#prerequisites)
> 2. **Create your project.** [Start from the template](#start-from-the-template)
> 3. **Build and run it.** [Build and run](#build-and-run)
> 4. **Read what you got.** [What is in the template](#what-is-in-the-template)
> 5. **Move to a newer Nodens.** [Upgrading](#upgrading-nodens)
> 6. **Change Nodens itself.** [Working on the framework](#working-on-the-framework)

> ⚠️ **Linux only.** The published development image builds and validates Nodens applications on Linux. Native Windows and macOS builds are not supported. See [Building and Toolchain](./building-and-toolchain.md).

## Prerequisites

Install Docker and an editor with Dev Container support: [Zed](https://zed.dev/), or [Visual Studio Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).

Everything else comes in one published image, `ghcr.io/eldritch-codex/arch-dev:main`: the C++ toolchain, the X11 and Wayland development files that GLFW builds against, OpenGL and Vulkan tools and shader tooling. The template and the Nodens repository each ship two Dev Container configurations, `nvidia` and `dri` (Intel and AMD); your editor asks which to open. NVIDIA hosts need the NVIDIA Container Toolkit first. See [Building and Toolchain](./building-and-toolchain.md) for both.

If you keep Zed on the host instead of reopening it in the container, install the Dev Containers CLI (`npm install -g @devcontainers/cli`). The tracked Zed tasks and debug scenarios use it to build, run and debug inside the container. See [Building and Toolchain](./building-and-toolchain.md).

## Start from the template

On the [Nodens Application Template](https://github.com/eldritch-codex/template-nodens-app), select **Use this template** to create your repository, open it in your editor, and reopen it in the devcontainer. The template's README has the details.

## Build and run

The devcontainer's default compiler is Clang, the one Nodens is built and tested with. Configure, build and run:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target NodensApp
./build/NodensApp
```

A window opens. That is a complete Nodens application.

## What is in the template

Two files matter: the `CMakeLists.txt` that fetches Nodens, and the application class.

```cmake
cmake_minimum_required(VERSION 3.30)

# CMake must know about experimental standard-library module support before
# project() enables the C++ language. Nodens requires this when consumed via
# FetchContent or add_subdirectory().
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(NodensApp VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(nodens
    GIT_REPOSITORY https://github.com/eldritch-codex/nodens.git
    GIT_TAG        v0.1.1 # A release tag; change it to move to a newer Nodens release
)
FetchContent_MakeAvailable(nodens)

add_executable(NodensApp src/main.cpp)
target_link_libraries(NodensApp PRIVATE Nodens::Nodens)
```

The `GIT_TAG` pins one Nodens release, so every build gets the same framework version.

The application subclasses `Nodens::Application` and describes itself with an `FApplicationConfig`:

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

The specification also has `VSync` and `GraphicsAPI` fields (the default API is Vulkan). To put your own code in the application, continue with [Creating a Custom Layer](./creating-a-custom-layer.md).

## Upgrading Nodens

Change the tag in `FetchContent_Declare` to a newer release from the [Nodens releases](https://github.com/eldritch-codex/nodens/releases) and rebuild. While Nodens is at 0.x, a release that changes the API bumps the minor version, so read the [changelog](../../CHANGELOG.md) before moving from 0.1 to 0.2.

## Working on the framework

Clone the Nodens repository only when you want to change the framework itself. It uses the same development image as applications, so both share one compiler, desktop, Vulkan and shader environment. Changes to that image are made in the [Devcontainers repository](https://github.com/eldritch-codex/devcontainers) and can be tested locally. Nodens keeps a single trunk, `main`; see [Branching and releases](../BRANCHING.md).
To develop an application and the framework together, replace `FetchContent` with a local checkout:

```cmake
add_subdirectory(path/to/Nodens)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE Nodens::Nodens)
```

`CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` must still come before `project()` in your own `CMakeLists.txt`, because Nodens cannot enable that toolchain feature after your project has enabled C++. This still builds Nodens from source; an installable CMake package is not provided.

## Next steps

- [Creating a Custom Layer](./creating-a-custom-layer.md): add your own behavior.
- [Architecture Overview](./architecture-overview.md): how the framework is structured.
- [Building and Toolchain](./building-and-toolchain.md): compilers, options, dependencies and troubleshooting.
