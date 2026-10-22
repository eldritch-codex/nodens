module;

#include <imgui.h>
#include <implot.h>
#include <implot3d.h>
#include <tracy/Tracy.hpp>

module Example.CircularWave3DLayer;

import nodens;
import std;

CircularWave3DLayer::CircularWave3DLayer() : ILayer("CircularWave3D")
{
    ZoneScoped;

    constexpr float kIncrement{0.001f};

    // Generate data for ImPlot
    for (int i = 0; i < kNumberOfPoints; i++)
    {
        t[i] = i * kIncrement;
        xs[i] = std::sin(m_Frequency * t[i]);
        ys[i] = std::cos(m_Frequency * t[i]);
    }
} // CircularWave3DLayer::CircularWave3DLayer

void CircularWave3DLayer::OnUpdate(Nodens::TimeStep previous_update_duration)
{
    ZoneScoped;

    constexpr float kIncrement{0.001f};
    for (int i = 0; i < kNumberOfPoints; i++)
    {
        t[i] += kIncrement;
        xs[i] = std::sin(m_Frequency * t[i]);
        ys[i] = std::cos(m_Frequency * t[i]);
    }

    // Nodens::ClientLogger().info("Updated CircularWave3DLayer with timestep: {:.3f} ms",
    //                             previous_update_duration.GetMilliseconds());
} // CircularWave3DLayer::OnUpdate

void CircularWave3DLayer::OnImGuiRender(Nodens::TimeStep ts)
{
    ZoneScoped;

    // Implot
    ImGui::Begin("ImPlot Example");
    if (ImPlot::BeginPlot("Wave components", ImVec2(-1, -1)))
    {
        ImPlot::SetupAxes("t", "x-y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("Sine", t, xs, kNumberOfPoints);
        ImPlot::PlotLine("Cosine", t, ys, kNumberOfPoints);
        ImPlot::EndPlot();
    }
    ImGui::End();

    // Implot3D
    ImGui::Begin("ImPlot3D Example");
    if (ImPlot3D::BeginPlot("3D Circular Wave", ImVec2(-1, -1)))
    {
        ImPlot3D::SetupAxes("t",
                            "x",
                            "y",
                            ImPlot3DAxisFlags_AutoFit,
                            ImPlot3DAxisFlags_AutoFit,
                            ImPlot3DAxisFlags_AutoFit);
        ImPlot3D::PlotLine("Wave", t, xs, ys, kNumberOfPoints);
        ImPlot3D::EndPlot();
    }
    ImGui::End();
} // CircularWave3DLayer::OnImGuiRender
