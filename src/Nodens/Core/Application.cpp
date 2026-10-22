/// @file Application.cpp
/// @brief Implementation of the Application class - construction, main loop,
/// and event dispatch.
/// @ingroup Core

module;

#include <tracy/Tracy.hpp>

module Nodens.Application;

import Nodens.TimeStep;
import Nodens.Event;
import Nodens.LayerStack;
import Nodens.Log;
import Nodens.Window;
import Nodens.ImGuiRenderer;
import Nodens.ImGuiLayer;
#if defined(ND_HAS_OPENGL)
import Nodens.OpenGLImGuiRenderer;
#endif
#if defined(ND_HAS_VULKAN)
import Nodens.VulkanContext;
import Nodens.VulkanImGuiRenderer;
#endif
import std;

namespace Nodens
{

Application* Application::s_Instance = nullptr;

Application::Application(const FApplicationSpecification& specification)
    : m_Specification(specification)
{
    ZoneScoped;

    // Ensure strictly one Application instance exists
    if (s_Instance)
        FatalCore("Application already exists!");
    s_Instance = this;

    m_JobSystem = std::make_unique<JobSystem>();
    m_LayerStack = std::make_unique<LayerStack>();
    m_EventBus = std::make_unique<EventBus>();

    FWindowProps props{
        .Title = m_Specification.Name,
        .Width = m_Specification.WindowWidth,
        .Height = m_Specification.WindowHeight,
        .VSync = m_Specification.VSync,
        .API = m_Specification.GraphicsAPI,
    };
    m_Window = std::unique_ptr<IWindow>(IWindow::Create(props));
    m_Window->SetInputEventCallback([this](RoutedInputEvent& event) { OnInputEvent(event); });

    if (m_Specification.EnableGUI)
    {
        std::shared_ptr<ImGuiRenderer> imguiRenderer;
        bool rendererCreated{false};
#if defined(ND_HAS_OPENGL)
        if (m_Specification.GraphicsAPI == EGraphicsAPI::OpenGL)
        {
            imguiRenderer = std::make_shared<OpenGLImGuiRenderer>();
            rendererCreated = true;
        }
#endif
#if defined(ND_HAS_VULKAN)
        if (!rendererCreated && m_Specification.GraphicsAPI == EGraphicsAPI::Vulkan)
        {
            // Borrow the window's initialized context; the ImGui backend must not create a second
            // device.
            auto* context = dynamic_cast<VulkanContext*>(m_Window->GetGraphicsContext());
            if (!context)
                FatalCore("Vulkan window has no Vulkan graphics context!");
            imguiRenderer = std::make_shared<VulkanImGuiRenderer>(*context);
            rendererCreated = true;
        }
#endif
        if (!rendererCreated)
            FatalCore("ImGui renderer is not available for this graphics API!");

        m_ImGuiLayer = new ImGuiLayer{imguiRenderer, m_Specification.DefaultTheme};
        m_ImGuiLayer->BlockEvents(m_Specification.ShouldImGuiBlockInputs);
        PushOverlay(m_ImGuiLayer);
    }
}

Application::~Application()
{
    ZoneScoped;
    // Cleanup logic if necessary
}

void Application::PushLayer(ILayer* layer)
{
    ZoneScoped;
    m_LayerStack->PushLayer(layer);
    layer->OnAttach();
}

void Application::PushOverlay(ILayer* overlay)
{
    ZoneScoped;
    m_LayerStack->PushOverlay(overlay);
    overlay->OnAttach();
}

IWindow& Application::GetWindow()
{
    return *m_Window;
}

JobSystem& Application::GetJobSystem()
{
    return *m_JobSystem;
}

EventBus& Application::GetEventBus()
{
    return *m_EventBus;
}

const FApplicationSpecification& Application::GetSpecification() const
{
    return m_Specification;
}

Application& Application::Get()
{
    return *s_Instance;
}

void Application::Run()
{
    const auto startTime = std::chrono::steady_clock::now();

    while (m_Running)
    {
        ZoneScoped;

        const auto currentTime = std::chrono::steady_clock::now();
        float time = std::chrono::duration<float>(currentTime - startTime).count();
        TimeStep timestep = time - m_LastFrameTime;
        m_LastFrameTime = time;

        // Flush queued non-input events
        m_EventBus->Flush();

#if defined(ND_HAS_VULKAN)
        if (m_Specification.GraphicsAPI == EGraphicsAPI::Vulkan)
        {
            // Acquire once before layers run so Nyar and ImGui append to the same command buffer.
            auto* context = dynamic_cast<VulkanContext*>(m_Window->GetGraphicsContext());
            if (!context)
                FatalCore("Vulkan window has no Vulkan graphics context!");
            context->BeginFrame();
        }
#endif

        // Update each layer
        for (ILayer* layer : *m_LayerStack)
            layer->OnUpdate(timestep);

        if (m_ImGuiLayer)
        {
            m_ImGuiLayer->Begin();
            for (ILayer* layer : *m_LayerStack)
                layer->OnImGuiRender(timestep);
            m_ImGuiLayer->End();
        }

        if (m_Window)
            m_Window->OnUpdate();

        FrameMark;
    }
}

bool Application::OnWindowClose(InputEvents::WindowClose& e)
{
    m_Running = false;
    return false; // Do not consume the event. Let other layers know that the application will be
                  // closed.
}

void Application::OnInputEvent(RoutedInputEvent& e)
{
    ZoneScoped;

    InputEventDispatcher dispatcher(e);
    dispatcher.Dispatch<InputEvents::WindowClose>([this](InputEvents::WindowClose& event)
                                                  { return OnWindowClose(event); });

    // Run through LayerStack from last to first
    for (auto layer : *m_LayerStack | std::views::reverse)
    {
        layer->OnInputEvent(e);
        if (e.Handled)
            break;
    }
}

} // namespace Nodens
