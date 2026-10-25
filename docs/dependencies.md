<!-- covers: cmake/, vendor/glad/, README.md -->
# Dependencies

Every third-party library is declared in `cmake/NodensDependencies.cmake` and pinned with `GIT_TAG`. No declaration sets `FIND_PACKAGE_ARGS`, so a system-installed copy is never consulted: all five libraries are fetched at configure time.

## The ImGui stack

ImGui, ImPlot and ImPlot3D have no CMake build upstream, so `cmake/NodensImGui.cmake` creates the `ImGui` target from the fetched sources, with `CXX_MODULE_STD OFF`.

On Linux the target links X11. Configuration requires the Wayland client header (`wayland-client.h`) and the `wayland-scanner` program, and fails without them.

## Vendored and optional pieces

glad lives in `vendor/glad` and is added only when `ND_ENABLE_OPENGL` is on.

`TRACY_ENABLE` is forced to `OFF` in the cache. Only the `Profiling` configuration defines it, on the `TracyClient` target.

## Changing a pin

Edit that library's `GIT_TAG` in `cmake/NodensDependencies.cmake`, then update the version table in `README.md`.
