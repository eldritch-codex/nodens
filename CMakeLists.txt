cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
project(Nodens VERSION 0.1.0 LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Core Build Settings
# -----------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profiling" CACHE
  STRING "" FORCE)

set(CMAKE_CXX_FLAGS_PROFILING
  "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-omit-frame-pointer"
  CACHE STRING "Profiling build flags" FORCE)
set(CMAKE_C_FLAGS_PROFILING
  "${CMAKE_C_FLAGS_RELEASE} -g -fno-omit-frame-pointer"
  CACHE STRING "Profiling build flags" FORCE)
# Note that `-fno-omit-frame-pointer` is only for GCC/Clang. When adding MSVC
# support we should tweak this.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Developer Options (primary touchpoints)
option(ND_ENABLE_OPENGL "Build the OpenGL backend" ON)
option(ND_ENABLE_VULKAN "Build the Vulkan backend when Vulkan is available" ON)
option(ND_BUILD_EXAMPLES
  "Build the Nodens example applications" ON)
option(ND_BUILD_TRACY_PROFILER
  "Build the Tracy Profiler GUI from Tracy source" OFF)

if(NOT PROJECT_IS_TOP_LEVEL)
  option(ND_BUILD_EXAMPLES "Build the Nodens example applications" OFF)
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(ND_ENABLE_OPENGL)
  add_subdirectory(vendor/glad)
endif()
include(NodensDependencies)

if(ND_BUILD_TRACY_PROFILER)
  include(ExternalProject)
  ExternalProject_Add(tracy-profiler
    SOURCE_DIR "${Tracy_SOURCE_DIR}/profiler"
    BINARY_DIR "${Tracy_BINARY_DIR}/tracy-profiler"
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/tracy-profiler
    UPDATE_COMMAND ""
    EXCLUDE_FROM_ALL TRUE
  )
endif()

if(ND_ENABLE_VULKAN)
  find_package(Vulkan QUIET)
endif()
include(NodensImGui)

if(ND_ENABLE_VULKAN AND Vulkan_FOUND)
  target_sources(ImGui PRIVATE
        "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
    )
  target_link_libraries(ImGui PUBLIC Vulkan::Vulkan)
endif()

set(ND_OPENGL_MODULES)
set(ND_OPENGL_IMPLEMENTATION_UNITS)
if(ND_ENABLE_OPENGL)
  list(APPEND ND_OPENGL_MODULES
        src/Platform/OpenGL/OpenGLContext.cppm
        src/Platform/OpenGL/OpenGLImGuiRenderer.cppm
    )
  list(APPEND ND_OPENGL_IMPLEMENTATION_UNITS
        src/Platform/OpenGL/OpenGLContext.cpp
        src/Platform/OpenGL/OpenGLImGuiRenderer.cpp
    )
endif()

set(ND_VULKAN_MODULES)
# Dear ImGui's Vulkan backend has a conventional implementation unit alongside its module.
set(ND_VULKAN_IMPLEMENTATION_UNITS)
if(ND_ENABLE_VULKAN AND Vulkan_FOUND)
  if(NOT TARGET Vulkan::cppm)
    add_library(NodensVulkanCppModule STATIC)
    add_library(Vulkan::cppm ALIAS NodensVulkanCppModule)

    target_compile_definitions(NodensVulkanCppModule PUBLIC
            VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
            VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
            VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING=1
        )
    target_include_directories(NodensVulkanCppModule PRIVATE
            "${Vulkan_INCLUDE_DIR}"
        )
    target_link_libraries(NodensVulkanCppModule PUBLIC Vulkan::Vulkan)
    set_target_properties(NodensVulkanCppModule PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
        )
    target_sources(NodensVulkanCppModule PUBLIC
            FILE_SET vulkan_modules TYPE CXX_MODULES
            BASE_DIRS "${Vulkan_INCLUDE_DIR}"
            FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
        )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(NodensVulkanCppModule PRIVATE
                -Wno-reserved-module-identifier
                -Wno-include-angled-in-module-purview
            )
    endif()
  endif()

  list(APPEND ND_VULKAN_MODULES
        src/Platform/Vulkan/VulkanContext.cppm
        src/Platform/Vulkan/VulkanImGuiRenderer.cppm
    )
  list(APPEND ND_VULKAN_IMPLEMENTATION_UNITS
        src/Platform/Vulkan/VulkanContext.cpp
        src/Platform/Vulkan/VulkanImGuiRenderer.cpp
    )
endif()

# -----------------------------------------------------------------------------
# Developer Touchpoint: Framework Source Lists
# -----------------------------------------------------------------------------
set(ND_FRAMEWORK_IMPLEMENTATION_UNITS
    # Core
    src/Nodens/Core/Application.cpp
    # Layers
    src/Nodens/Layers/Layer.cpp
    src/Nodens/Layers/LayerStack.cpp
    # Events
    src/Nodens/Events/EventBus.cpp
    # Concurrency
    src/Nodens/Concurrency/JobSystem.cpp
    # ImGui
    src/Nodens/ImGui/ImGuiLayer.cpp
    src/Nodens/ImGui/DefaultTheme.cpp
    # Platform
    ${ND_OPENGL_IMPLEMENTATION_UNITS}
    ${ND_VULKAN_IMPLEMENTATION_UNITS}
    src/Platform/GLFW/GlfwInput.cpp
    src/Platform/GLFW/GlfwWindow.cpp
)

set(ND_FRAMEWORK_MODULE_INTERFACES
    # Core
    src/Nodens/Core/Application.cppm
    src/Nodens/Core/Log.cppm
    src/Nodens/Core/TimeStep.cppm
    # Layers
    src/Nodens/Layers/Layer.cppm
    src/Nodens/Layers/LayerStack.cppm
    # Events
    src/Nodens/Events/Event.cppm
    src/Nodens/Events/EventBus.cppm
    src/Nodens/Events/InputEvents.cppm
    # Concurrency
    src/Nodens/Concurrency/JobSystem.cppm
    # ImGui
    src/Nodens/ImGui/ImGuiRenderer.cppm
    src/Nodens/ImGui/ImGuiLayer.cppm
    src/Nodens/ImGui/DefaultTheme.cppm
    # Input
    src/Nodens/Input/Input.cppm
    src/Nodens/Input/KeyCodes.cppm
    src/Nodens/Input/MouseButtonCodes.cppm
    # Rendering
    src/Nodens/Rendering/GraphicsContext.cppm
    src/Nodens/Rendering/Window.cppm
    # Platform
    ${ND_OPENGL_MODULES}
    # Optional Vulkan platform context and ImGui renderer
    ${ND_VULKAN_MODULES}
    # Umbrella module
    src/Nodens/nodens.cppm
)

# -----------------------------------------------------------------------------
# Framework Target
# -----------------------------------------------------------------------------
add_library(Nodens STATIC ${ND_FRAMEWORK_IMPLEMENTATION_UNITS})
add_library(Nodens::Nodens ALIAS Nodens)

target_sources(Nodens
    PUBLIC
        FILE_SET nodens_modules TYPE CXX_MODULES
        FILES ${ND_FRAMEWORK_MODULE_INTERFACES}
)

target_compile_features(Nodens PUBLIC cxx_std_23)

target_compile_definitions(Nodens PUBLIC
    $<$<CONFIG:Debug>:ND_DEBUG>
    $<$<CONFIG:Release,Profiling>:ND_RELEASE>
    SPDLOG_USE_STD_FORMAT
)

if(ND_ENABLE_OPENGL)
  target_compile_definitions(Nodens PRIVATE ND_HAS_OPENGL)
endif()
if(ND_ENABLE_VULKAN AND Vulkan_FOUND)
  target_compile_definitions(Nodens PRIVATE ND_HAS_VULKAN)
  target_link_libraries(Nodens PUBLIC Vulkan::cppm)
endif()

target_link_libraries(Nodens
    PUBLIC
        spdlog::spdlog
        glfw
        ImGui
        ImPlot
        ImPlot3d
        TracyClient
)
if(ND_ENABLE_OPENGL)
  target_link_libraries(Nodens PUBLIC glad)
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(ND_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# -----------------------------------------------------------------------------
# Documentation
# -----------------------------------------------------------------------------
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  add_custom_target(nodens-docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
endif()
