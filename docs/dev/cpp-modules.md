<!-- covers: CMakeLists.txt, src/Nodens/nodens.cppm, src/Nodens/Layers/ -->
# C++ modules and the build

Nodens publishes its API as module interfaces, and a consumer writes one line to reach all of it: `import nodens;`. `src/Nodens/nodens.cppm` re-exports every public submodule with one `export import Nodens.X;` line each.

## The decision

Each module is two files:

- An interface unit, such as `src/Nodens/Layers/LayerStack.cppm`, which declares `export module Nodens.LayerStack;`.
- An implementation unit, such as `src/Nodens/Layers/LayerStack.cpp`, which opens with `module Nodens.LayerStack;` and no `export`.

`Nodens` is a static library built from the implementation units. The interfaces are published through a `CXX_MODULES` file set named `nodens_modules`, both declared in `CMakeLists.txt`.

`import std` is enabled for the framework: configure sets the experimental identifier it needs and switches `CMAKE_CXX_MODULE_STD` on, near the top of `CMakeLists.txt`. The vendored ImGui target is built with `CXX_MODULE_STD OFF`, since it is a plain C++ library.

## Why

A module interface states the API boundary in the language rather than in a directory convention, and the compiler enforces it: what the interface does not export cannot be named by a consumer. The umbrella module adds one more layer, so an application does not track which submodule a name lives in.

`import std` removes the header preamble from every implementation unit and gives one standard-library module per build. Third-party headers do not have that luxury, so a header such as `GLFW/glfw3.h` goes in the global module fragment, between `module;` and the `export module` line, as in `src/Platform/OpenGL/OpenGLBackend.cppm`.

## What it costs

- **A toolchain, not just a compiler.** Modules need CMake 3.30 or newer, Ninja to order the dependency scan, and a recent Clang; the development image supplies all three.
- **A constraint on consumers.** `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` must be set before the consuming project's `project(... LANGUAGES CXX)` call, because the feature cannot be enabled once C++ is on. [Getting Started](../guide/getting-started.md) carries the two lines that do it.
- **Two lists per module.** A new module is registered in two places in `CMakeLists.txt`, added to the umbrella module, and its API page regenerated.

## Changing it

1. Add the implementation unit to `ND_FRAMEWORK_IMPLEMENTATION_UNITS` in `CMakeLists.txt`.
2. Add the interface to `ND_FRAMEWORK_MODULE_INTERFACES` in the same file.
3. Re-export the module from `src/Nodens/nodens.cppm`.
4. Regenerate the API docs with the `nodens-docs` target, which exists when Doxygen is found.
