/// @file nodens.cppm
/// @brief Umbrella module for the Nodens framework.
/// @details Importing this single module (`import nodens;`) brings in every public submodule
/// of the framework - Core, Events, Concurrency, Input, ImGui, and Rendering - so that
/// client applications never need to import individual modules.
///
/// @defgroup Core Core
/// @brief Application lifecycle, layer stack, logging, and timing utilities.
///
/// @defgroup Events Events
/// @brief Synchronous event type hierarchy, dispatcher, and asynchronous publish/subscribe bus.
///
/// @defgroup Concurrency Concurrency
/// @brief Multithreaded job system built on C++20 jthreads.
///
/// @defgroup Input Input
/// @brief Platform-agnostic keyboard and mouse input polling.
///
/// @defgroup ImGui ImGui
/// @brief Dear ImGui integration layer, renderer abstraction, and default themes.
///
/// @defgroup Rendering Rendering
/// @brief Graphics context and window abstractions.
///
/// @defgroup Platform Platform
/// @brief Concrete platform implementations (GLFW, OpenGL).

export module nodens;

export import Nodens.Application;
export import Nodens.Event;
export import Nodens.EventBus;
export import Nodens.InputEvents;
export import Nodens.GraphicsContext;
export import Nodens.ImGuiLayer;
export import Nodens.ImGuiRenderer;
export import Nodens.Input;
export import Nodens.JobSystem;
export import Nodens.KeyCodes;
export import Nodens.Layer;
export import Nodens.LayerStack;
export import Nodens.Log;
export import Nodens.MouseButtonCodes;
export import Nodens.TimeStep;
export import Nodens.DefaultTheme;
export import Nodens.Window;
