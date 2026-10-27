/// @file GraphicsBackend.cppm
/// @brief Abstract interface for a graphics rendering backend.
/// @ingroup Rendering

export module Nodens.GraphicsBackend;

export namespace Nodens
{
/// @brief Abstract base class representing a GPU rendering backend.
/// @details An IGraphicsBackend encapsulates backend initialization and frame presentation
///          for a specific graphics API. The concrete implementation is created by the
///          platform window or an external renderer during initialization.
///          A Vulkan implementation may own its instance, device, swapchain, and
///          synchronization without exposing Vulkan types through this interface.
/// @see OpenGLBackend, IWindow
/// @ingroup Rendering
class IGraphicsBackend
{
public:
    virtual ~IGraphicsBackend() = default;

    /// @brief Initializes backend state and resources.
    virtual void Init() = 0;

    /// @brief Presents the completed frame when the backend owns presentation.
    /// @details OpenGL swaps its window buffers. Vulkan implementations may submit
    ///          and present through their swapchain or delegate presentation to a renderer.
    virtual void Present() = 0;
};
} // namespace Nodens
