<!-- covers: CMakeLists.txt, src/Nodens/nodens.cppm, src/Nodens/Layers/ -->
# C++ modules and the build

Nodens publishes its API as module interfaces. A consumer writes a single `import nodens;`, and `src/Nodens/nodens.cppm` re-exports every public submodule with one `export import Nodens.X;` line each.

## The shape of a module

Each module is two files:

- An interface unit, such as `src/Nodens/Layers/LayerStack.cppm`, declares `export module Nodens.LayerStack;`.
- An implementation unit, such as `src/Nodens/Layers/LayerStack.cpp`, opens with `module Nodens.LayerStack;` and no `export`.

`Nodens` is a static library built from the implementation units. The interfaces are published through a `CXX_MODULES` file set named `nodens_modules`, both declared in `CMakeLists.txt`.

## import std

Configure sets the experimental UUID that `import std` needs and switches `CMAKE_CXX_MODULE_STD` on, both near the top of `CMakeLists.txt`.

## Third-party headers

The `ImGui` target is built with `CXX_MODULE_STD OFF`.

A header such as `GLFW/glfw3.h` goes in the global module fragment, between `module;` and the `export module` line, as in `src/Platform/OpenGL/OpenGLContext.cppm`.

## Adding a module

1. Add the implementation unit to `ND_FRAMEWORK_IMPLEMENTATION_UNITS` in `CMakeLists.txt`.
2. Add the interface to `ND_FRAMEWORK_MODULE_INTERFACES` in the same file.
3. Re-export the module from `src/Nodens/nodens.cppm`.
4. Regenerate the API docs with the `nodens-docs` target, which exists when Doxygen is found.
