<!-- covers: CMakeLists.txt, src/Nodens/Rendering/, src/Nodens/ImGui/, src/Nodens/Core/Application.cpp, src/Platform/ -->
# Graphics backends

Nodens does not have a renderer. It has a window that owns a graphics backend, and two interfaces through which everything else reaches it. The choice between OpenGL and Vulkan is made twice: once when the framework is built, and once when an application starts.

## The decision

- `IGraphicsBackend` in `src/Nodens/Rendering/GraphicsBackend.cppm` covers initialization and presentation, and nothing else.
- `ImGuiRenderer` in `src/Nodens/ImGui/ImGuiRenderer.cppm` covers ImGui's draw data.
- Every backend under `src/Platform/` implements both.
- `FWindowConfig` defaults `GraphicsAPI` to `EGraphicsAPI::Vulkan`, and `Application` creates the matching ImGui renderer at startup. When no branch matches, `FatalCore` reports that no ImGui renderer exists for that graphics API.

## Why

Two forces shaped it.

**The renderer is outside the framework.** Nyar, and any other renderer, borrows the window's backend instead of owning one, so the window must expose an interface that does not name an API. A single `IGraphicsBackend` with `Init` and `Present` is the smallest such interface.

**The two choices answer different questions.** Building a backend is about what a machine compiles and whether the SDK was present; selecting one is about which GPU the application should use at runtime. Keeping them apart means a single build can start on either API, and a build without an SDK still produces a working framework.

## What it costs

- **A new API touches four places:** implement `IGraphicsBackend`, implement `ImGuiRenderer`, add an `EGraphicsAPI` value and teach `GlfwWindow` to create the window and the backend, and teach `Application` to pick the matching ImGui renderer.
- **A mismatch fails at startup, not at compile time.** When `ND_ENABLE_VULKAN` is on and the SDK is missing, CMake only warns, so the build succeeds without the backend and an application using the default API exits with `Nodens was built without Vulkan support!`.
- **The compile definitions are private.** `ND_HAS_OPENGL` and `ND_HAS_VULKAN` belong to the `Nodens` target, so a consumer cannot test them. The only check available is the compile database, which is what CI does: `grep -c -- '-DND_HAS_VULKAN' build/compile_commands.json` must not be zero.

## Changing it

1. Implement `IGraphicsBackend` for the new API under `src/Platform/`.
2. Implement `ImGuiRenderer` for it, since ImGui draws through the API rather than through the framework.
3. Add the value to `EGraphicsAPI`, and teach `GlfwWindow` to create the window and the backend for it.
4. Teach `Application` which ImGui renderer that value needs.
5. Add the build switch beside `ND_ENABLE_OPENGL` and `ND_ENABLE_VULKAN` in `CMakeLists.txt`, and its compile definition for the `Nodens` target.

`OpenGLBackend::Init` shows the smallest complete implementation: it makes the context current, loads the GL function pointers through GLAD and sets the default GL state.
