<!-- covers: CMakeLists.txt, src/Nodens/Rendering/, src/Nodens/ImGui/, src/Nodens/Core/Application.cpp, src/Platform/ -->
# Graphics backends

Nodens puts its graphics API behind two interfaces: `IGraphicsBackend` (`src/Nodens/Rendering/GraphicsBackend.cppm`) for the rendering backend, and `ImGuiRenderer` (`src/Nodens/ImGui/ImGuiRenderer.cppm`) for ImGui's draw data. Each backend under `src/Platform/` implements both.

## Choosing a backend

`FWindowConfig` in `src/Nodens/Rendering/Window.cppm` defaults `GraphicsAPI` to `EGraphicsAPI::Vulkan`.

## Renderer selection

`Application` (`src/Nodens/Core/Application.cpp`) picks the ImGui renderer at startup:

- When `ND_HAS_OPENGL` is defined and the configuration asks for OpenGL, it creates an `OpenGLImGuiRenderer`.
- When `ND_HAS_VULKAN` is defined and it asks for Vulkan, it creates a `VulkanImGuiRenderer`.
- When neither branch created one, `FatalCore` reports that the ImGui renderer is not available for this graphics API.

## Backend ownership

`OpenGLBackend::Init` makes the context current, loads the GL function pointers through GLAD and sets the default GL state.

## Build switches

`ND_ENABLE_OPENGL` and `ND_ENABLE_VULKAN` in `CMakeLists.txt` decide which backends are built; Vulkan is built only when it is available. They define `ND_HAS_OPENGL` and `ND_HAS_VULKAN` for the `Nodens` target, and those definitions are what the code tests when it selects a renderer.

When `ND_ENABLE_VULKAN` is on and the Vulkan SDK is missing, CMake prints a configure warning that the build has no Vulkan backend and that applications using the default `GraphicsAPI` will exit at startup. CI turns that case into a failure: it checks that `-DND_HAS_VULKAN` is in `compile_commands.json`, so the build cannot silently become OpenGL only.
