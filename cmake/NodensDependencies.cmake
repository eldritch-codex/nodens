# =============================================================================
# NodensDependencies.cmake - Centralized dependency declarations for Nodens
# =============================================================================
# Uses FetchContent with FIND_PACKAGE_ARGS where possible so that:
#   1. System-installed packages are preferred (via find_package).
#   2. If not found, source is downloaded and built automatically.
#
# Libraries WITHOUT official CMake packaging (ImGui, ImPlot, ImPlot3d)
# are always fetched from source. Their build targets are created
# separately in NodensImGui.cmake.
# =============================================================================

include(FetchContent)

# ─── spdlog ──────────────────────────────────────────────────────────────────
set(SPDLOG_ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
)

# ─── GLFW ────────────────────────────────────────────────────────────────────
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Build both Linux window-system backends into GLFW. Runtime selection is then
# handled by GLFW (or an explicit GLFW_PLATFORM hint) rather than being fixed
# by the build configuration.
set(GLFW_BUILD_X11     ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# ─── Tracy ───────────────────────────────────────────────────────────────────
# This disables the Tracy profiler by default, preventing the profiler from
# being linked into the build unless explicitly enabled later.
set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG        v0.14.1
    GIT_SHALLOW    TRUE
)

# ─── ImGui (no find_package - always from source) ───────────────────────────
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.9b-docking
    GIT_SHALLOW    TRUE
)

# ─── ImPlot (no find_package - always from source) ──────────────────────────
FetchContent_Declare(implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v1.0
    GIT_SHALLOW    TRUE
)

# ─── ImPlot3d (no find_package - always from source) ────────────────────────
FetchContent_Declare(implot3d
    GIT_REPOSITORY https://github.com/brenocq/implot3d.git
    GIT_TAG        v0.4
    GIT_SHALLOW    TRUE
)

# ─── Populate all dependencies ──────────────────────────────────────────────
# spdlog, glfw, tracy: find_package attempted first, fallback to source.
# imgui, implot, implot3d: always downloaded (no CMake packaging upstream).
FetchContent_MakeAvailable(spdlog tracy imgui implot implot3d)

# Enable Tracy instrumentation only for the `Profiling` build config.
# Set to PUBLIC so TracyClient.cpp itself and every consumer (Nodens, examples)
# agree on the macro definition.
target_compile_definitions(TracyClient PUBLIC
    $<$<CONFIG:Profiling>:TRACY_ENABLE>)
