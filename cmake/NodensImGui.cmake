# =============================================================================
# NodensImGui.cmake - Build ImGui, ImPlot, and ImPlot3d from FetchContent
# sources
# =============================================================================
# These libraries have no official CMake build system. We create targets from
# the source directories populated by FetchContent in NodensDependencies.cmake.
#
# Targets are guarded with if(NOT TARGET ...) so that if a consumer has already
# defined these targets, we don't collide.
# =============================================================================

# ─── ImGui ───────────────────────────────────────────────────────────────────
if(NOT TARGET ImGui)
  set(ND_IMGUI_BACKEND_SOURCES
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )
  if(ND_ENABLE_OPENGL)
    list(APPEND ND_IMGUI_BACKEND_SOURCES
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
  endif()

  add_library(ImGui STATIC
        # Core
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp

        # Backends
        ${ND_IMGUI_BACKEND_SOURCES}
    )

  target_include_directories(ImGui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

  target_compile_definitions(ImGui PUBLIC GLFW_INCLUDE_NONE)
  target_link_libraries(ImGui PUBLIC glfw)
  if(ND_ENABLE_OPENGL)
    target_compile_definitions(ImGui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
    target_link_libraries(ImGui PUBLIC glad)
  endif()

  set_target_properties(ImGui PROPERTIES CXX_MODULE_STD OFF)

  # Nodens currently targets Linux desktop environments. The GLFW backend
  # and Dear ImGui's docking backend are built with both X11 and Wayland
  # support; the active backend is selected at runtime.
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Dear ImGui's GLFW docking backend includes X11 headers and uses X11
    # facilities for X11 multi-viewport support. This is a compile-time
    # dependency even when the application is later run under Wayland.
    find_package(X11 REQUIRED)

    if(TARGET X11::X11)
      target_link_libraries(ImGui PUBLIC X11::X11)
    else()
      target_include_directories(ImGui PUBLIC ${X11_INCLUDE_DIR})
      target_link_libraries(ImGui PUBLIC ${X11_LIBRARIES})
    endif()

    # Fail during configuration instead of silently compiling an incomplete
    # backend when the Wayland development files are missing. GLFW itself
    # consumes wayland-scanner while configuring its Wayland backend.
    find_path(WAYLAND_CLIENT_INCLUDE_DIR
      NAMES wayland-client.h
      REQUIRED
    )
    find_program(WAYLAND_SCANNER_EXECUTABLE
      NAMES wayland-scanner
      REQUIRED
    )

    target_include_directories(ImGui PRIVATE
      ${WAYLAND_CLIENT_INCLUDE_DIR}
    )
  endif()
endif()

# ─── ImPlot ──────────────────────────────────────────────────────────────────
if(NOT TARGET ImPlot)
  add_library(ImPlot STATIC
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
    )

  target_include_directories(ImPlot PUBLIC ${implot_SOURCE_DIR})
  target_link_libraries(ImPlot PUBLIC ImGui)

  set_target_properties(ImPlot PROPERTIES CXX_MODULE_STD OFF)
endif()

# ─── ImPlot3d ────────────────────────────────────────────────────────────────
if(NOT TARGET ImPlot3d)
  add_library(ImPlot3d STATIC
        ${implot3d_SOURCE_DIR}/implot3d.cpp
        ${implot3d_SOURCE_DIR}/implot3d_items.cpp
    )

  target_include_directories(ImPlot3d PUBLIC ${implot3d_SOURCE_DIR})
  target_link_libraries(ImPlot3d PUBLIC ImGui)

  set_target_properties(ImPlot3d PROPERTIES CXX_MODULE_STD OFF)
endif()
