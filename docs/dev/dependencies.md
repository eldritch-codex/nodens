<!-- covers: cmake/, vendor/glad/, README.md -->
# Dependencies

Every third-party library is declared in `cmake/NodensDependencies.cmake` and pinned with `GIT_TAG`. No declaration sets `FIND_PACKAGE_ARGS`, so a system-installed copy is never consulted: all six libraries are fetched at configure time.

## The decision

- One file holds every pin, so the versions a build uses are visible in one place.
- Nothing is found on the system. A library that CMake cannot fetch is a configure error, not a fallback to whatever the distribution installed.
- ImGui, ImPlot and ImPlot3D have no CMake build upstream, so `cmake/NodensImGui.cmake` creates the `ImGui` target from the fetched sources, with `CXX_MODULE_STD OFF`.
- glad is vendored in `vendor/glad` and added only when `ND_ENABLE_OPENGL` is on.
- `TRACY_ENABLE` is forced to `OFF` in the cache. Only the `Profiling` configuration defines it, on the `TracyClient` target.

## Why

**A build that reproduces from the repository alone.** A pin fixes a commit, and a fetch at that commit produces the same sources on every machine. A system copy brings whatever flags, patches and version the distribution chose, which the project would then have to detect and support.

**The ImGui stack needs a specific revision.** The docking branch and the exact ImGui, ImPlot and ImPlot3D combination are part of the framework's behaviour, not an incidental dependency. Building them here keeps that combination in the repository rather than in a distribution's packaging decision.

**Modules need a consistent standard.** The framework imports `std` and publishes module interfaces. A third-party library built against a different standard-library configuration cannot be dropped into that build, so each dependency is compiled by this build with its own settings, such as `CXX_MODULE_STD OFF` for ImGui.

## What it costs

- **Configuration needs network access.** The first configure fetches six repositories; offline work starts from a populated build directory or a local mirror.
- **A pin is updated by hand.** Moving a version is an edit to one `GIT_TAG`, plus the revision table in the build page and the table in `README.md`, which repeat the same versions.
- **Two Linux headers are build requirements.** The ImGui target links X11, and configuration requires the Wayland client header and `wayland-scanner`, so a machine without them fails before any framework source compiles.
- **glad tracks the OpenGL version.** The vendored loader is regenerated when the OpenGL level Nodens targets changes, which is a manual step outside CMake.

## Changing a pin

1. Edit that library's `GIT_TAG` in `cmake/NodensDependencies.cmake`.
2. Rebuild and run the examples, since a minor version can change an API the framework calls.
3. Update the revision table in [Building and Toolchain](../guide/building-and-toolchain.md) and the one in `README.md`.
4. Add the entry to the changelog when the change is visible to a consumer.
