/// @file ImGuiLayer.cpp
/// @brief Implementation of ImGuiLayer: context creation, frame bracketing, and event filtering.
/// @ingroup ImGui

module;

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <implot3d.h>
#include <tracy/Tracy.hpp>

module Nodens.ImGuiLayer;

import Nodens.Application;
import Nodens.DefaultTheme;
import Nodens.Window;
import Nodens.Log;
import std;

namespace Nodens
{

ImGuiLayer::ImGuiLayer(const std::shared_ptr<ImGuiRenderer>& renderer, EDefaultTheme theme)
    : ILayer("ImGuiLayer"), m_Renderer(renderer), m_Theme(theme)
{
}

ImGuiLayer::~ImGuiLayer()
{
}

void ImGuiLayer::OnAttach()
{
    ZoneScoped;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImPlot3D::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (Application::Get().GetSpecification().GraphicsAPI == EGraphicsAPI::OpenGL)
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

    switch (m_Theme)
    {
        case EDefaultTheme::Dark:
            Nodens::ApplyDefaultDarkTheme();
            break;
        case EDefaultTheme::Light:
            Nodens::ApplyDefaultLightTheme();
            break;
        case EDefaultTheme::None:
            ImGui::StyleColorsDark();
            break;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Application& app = Application::Get();
    // We assume the native window is always GLFW for now, but we cast safely
    GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

    // Use the injected renderer to initialize
    if (m_Renderer)
        m_Renderer->Init(window);
}

void ImGuiLayer::OnDetach()
{
    ZoneScoped;

    if (m_Renderer)
        m_Renderer->Shutdown();

    ImPlot3D::DestroyContext();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{
    ZoneScoped;

    // Delegate NewFrame to the renderer (handles backend specific updates)
    if (m_Renderer)
        m_Renderer->NewFrame();

    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
}

void ImGuiLayer::End()
{
    ZoneScoped;

    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();

    // Keep DisplaySize in GLFW's logical coordinate space; Vulkan supplies the physical framebuffer
    // scale in its renderer so mouse coordinates and rasterization remain consistent.
    ImGui::Render();

    if (m_Renderer)
        m_Renderer->RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Note: UpdatePlatformWindows is generic ImGui logic
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        if (app.GetSpecification().GraphicsAPI == EGraphicsAPI::OpenGL)
            glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiLayer::OnImGuiRender(TimeStep ts)
{
}

void ImGuiLayer::OnInputEvent(RoutedInputEvent& e)
{
    if (m_BlockInputEvents)
    {
        ImGuiIO& io = ImGui::GetIO();
        e.Handled |= IsMouseEvent(e.Event) && io.WantCaptureMouse;
        e.Handled |= IsKeyboardEvent(e.Event) && io.WantCaptureKeyboard;
    }
}

} // namespace Nodens
