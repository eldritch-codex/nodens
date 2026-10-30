/// @file Application.cpp
/// @brief Implementation of the Application class - construction, main loop,
/// and event dispatch.
/// @ingroup Core

module;

#include <tracy/Tracy.hpp>

module Nodens.Application;

import Nodens.TimeStep;
import Nodens.FramePacer;
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
import Nodens.VulkanBackend;
import Nodens.VulkanImGuiRenderer;
#endif
import std;

namespace Nodens
{

Application* Application::s_Instance = nullptr;

Application::Application(const FApplicationConfig& config) : m_Config(config)
{
    ZoneScoped;

    // Ensure strictly one Application instance exists
    if (s_Instance)
        FatalCore("Application already exists!");
    s_Instance = this;

    m_TaskSystem = std::make_unique<TaskSystem>();
    m_LayerStack = std::make_unique<LayerStack>();
    m_EventBus = std::make_unique<EventBus>();
    m_Pacer = std::make_unique<FramePacer>(m_Config.Pacing);

    const bool wantsWindow = m_Config.Presentation != EPresentation::Headless;
    const bool wantsOverlay = m_Config.Presentation == EPresentation::WindowImGUI;

    if (wantsWindow)
    {
        FWindowConfig windowConfig{
            .Title = m_Config.Name,
            .Width = m_Config.WindowWidth,
            .Height = m_Config.WindowHeight,
            .VSync = m_Config.VSync,
            .GraphicsAPI = m_Config.GraphicsAPI,
        };
        m_Window = std::unique_ptr<IWindow>(IWindow::Create(windowConfig));
        m_Window->SetInputEventCallback([this](RoutedInputEvent& event) { OnInputEvent(event); });
    }

    // The overlay draws into a window, so it exists only in the state that has both.
    if (wantsOverlay)
    {
        std::shared_ptr<ImGuiRenderer> imguiRenderer;
        bool rendererCreated{false};
#if defined(ND_HAS_OPENGL)
        if (m_Config.GraphicsAPI == EGraphicsAPI::OpenGL)
        {
            imguiRenderer = std::make_shared<OpenGLImGuiRenderer>();
            rendererCreated = true;
        }
#endif
#if defined(ND_HAS_VULKAN)
        if (!rendererCreated && m_Config.GraphicsAPI == EGraphicsAPI::Vulkan)
        {
            // Borrow the window's initialized backend; the ImGui backend must not create a second
            // device.
            auto* backend = dynamic_cast<VulkanBackend*>(m_Window->GetBackend());
            if (!backend)
                FatalCore("Vulkan window has no Vulkan backend!");
            imguiRenderer = std::make_shared<VulkanImGuiRenderer>(*backend);
            rendererCreated = true;
        }
#endif
        if (!rendererCreated)
            FatalCore("ImGui renderer is not available for this graphics API!");

        m_ImGuiLayer = new ImGuiLayer{imguiRenderer, m_Config.Theme};
        m_ImGuiLayer->BlockEvents(m_Config.ShouldImGuiBlockInputs);
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

bool Application::HasWindow() const
{
    return m_Config.Presentation != EPresentation::Headless;
}

IWindow& Application::GetWindow()
{
    if (!m_Window)
        FatalCore("Application was configured without a window!");
    return *m_Window;
}

TaskSystem& Application::GetTaskSystem()
{
    return *m_TaskSystem;
}

EventBus& Application::GetEventBus()
{
    return *m_EventBus;
}

const FApplicationConfig& Application::GetConfig() const
{
    return m_Config;
}

Application& Application::Get()
{
    return *s_Instance;
}

void Application::Run()
{
    while (m_Running)
    {
        ZoneScoped;

        const FFrameTime frame = m_Pacer->BeginFrame();

        // Flush queued non-input events
        m_EventBus->Flush();

#if defined(ND_HAS_VULKAN)
        if (m_Window && m_Config.GraphicsAPI == EGraphicsAPI::Vulkan)
        {
            // Acquire once before layers run so Nyar and ImGui append to the same command buffer.
            auto* backend = dynamic_cast<VulkanBackend*>(m_Window->GetBackend());
            if (!backend)
                FatalCore("Vulkan window has no Vulkan backend!");
            backend->BeginFrame();
        }
#endif

        // Update each layer once per simulation step
        for (int step = 0; step < frame.Steps; ++step)
            for (ILayer* layer : *m_LayerStack)
                layer->OnUpdate(frame.Step);

        if (m_ImGuiLayer)
        {
            m_ImGuiLayer->Begin();
            for (ILayer* layer : *m_LayerStack)
                layer->OnImGuiRender(frame.Step);
            m_ImGuiLayer->End();
        }

        if (m_Window)
            m_Window->OnUpdate();

        m_Pacer->EndFrame();

        FrameMark;
    }
}

void Application::RequestStop()
{
    m_Running = false;
}

bool Application::OnWindowClose(InputEvents::WindowClose& e)
{
    RequestStop();
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
