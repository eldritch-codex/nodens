module;

#include <imgui.h>
#include <implot.h>
#include <implot3d.h>
#include <tracy/Tracy.hpp>

module Example.PlanetaryScanLayer;

import nodens;
import std;

PlanetaryScanLayer::PlanetaryScanLayer() : ILayer("PlanetaryScanLayer")
{
}

void PlanetaryScanLayer::OnAttach()
{
    auto subscriptionID =
        Nodens::Application::Get().GetEventBus().Subscribe<PlanetaryScanResultEvent>(
            [this](PlanetaryScanResultEvent& e) { AddResult(e); });
}

void PlanetaryScanLayer::OnUpdate(Nodens::TimeStep ts)
{
    m_TimePassed += ts;
}

void PlanetaryScanLayer::AddResult(const PlanetaryScanResultEvent& e)
{
    Nodens::ClientLogger().info("PlanetaryScan: received {}", e);

    std::lock_guard<std::mutex> lock(m_DataMutex);

    m_GalaxyDistances.push_back(e.m_Distance);
    m_GalaxyDensities.push_back(e.m_AtmosphereDensity);

    m_LatencyHistory.push_back(e.m_CalculationTime);
    if (e.m_CalculationTime > m_MaxLatency)
        m_MaxLatency = e.m_CalculationTime;

    // Keep history bounded so we don't run out of RAM eventually
    if (m_LatencyHistory.size() > 100)
    {
        m_LatencyHistory.erase(m_LatencyHistory.begin());
    }
}

static void LaunchScanJob(int id)
{
    // Push the heavy simulation work to a background thread
    Nodens::Application::Get().GetJobSystem().Submit(
        [id]()
        {
            ZoneScopedN("PlanetaryScanJob");

            // 1. Start Timer
            auto start = std::chrono::high_resolution_clock::now();

            // 2. SIMULATE WORK & GENERATE DATA
            // We use thread-local random engines for safety and speed
            static thread_local std::mt19937 generator(
                std::hash<std::thread::id>{}(std::this_thread::get_id()));

            // Randomize sleep (0.5s to 2.5s) to simulate variable workloads
            std::uniform_int_distribution<int> sleepDist(500, 2500);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(generator)));

            // Generate "Scientific Data"
            std::uniform_real_distribution<float> distDist(0.1f, 100.0f); // 0 to 100 Light Years
            std::normal_distribution<float> densityDist(0.5f, 0.15f);     // Atmosphere density

            PlanetaryScanResultEvent e{.m_ID = id};
            e.m_Distance = distDist(generator);
            e.m_AtmosphereDensity = densityDist(generator);

            // 3. Calculate Duration
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            e.m_CalculationTime = duration.count();

            // 4. Report Back via EventBus
            // This safely queues the event. Flush() will pick it up on the main thread later!
            Nodens::Application::Get().GetEventBus().QueueEvent<PlanetaryScanResultEvent>(e);
        });
}

void PlanetaryScanLayer::OnImGuiRender(Nodens::TimeStep ts)
{
    ZoneScoped;

    // ---------------------------------------------------------
    // MAIN CONTROL PANEL
    // ---------------------------------------------------------
    ImGui::Begin("Deep Space Analytics");
    ImGui::Text("Engine Time: %.2f s | FPS: %.1f", m_TimePassed, ImGui::GetIO().Framerate);

    if (ImGui::Button("Launch Single Probe"))
    {
        LaunchScanJob(++m_ScanCounter);
    }
    ImGui::SameLine();
    if (ImGui::Button("Launch Swarm (20 Probes)"))
    {
        for (int i = 0; i < 20; i++)
        {
            LaunchScanJob(++m_ScanCounter);
        }
    }
    ImGui::Separator();
    std::lock_guard<std::mutex> lock(m_DataMutex);
    if (ImPlot::BeginPlot("Galaxy Composition Analysis", ImVec2(-1, 0)))
    { // -1,0 fills available space
        ImPlot::SetupAxes("Distance (Light Years)",
                          "Atmosphere Density (g/cm^3)",
                          ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        if (!m_GalaxyDistances.empty())
        {
            ImPlot::PlotScatter("Planets",
                                m_GalaxyDistances.data(),
                                m_GalaxyDensities.data(),
                                (int)m_GalaxyDistances.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::End();

    // ---------------------------------------------------------
    // SECONDARY WINDOW
    // ---------------------------------------------------------
    ImGui::Begin("Performance Monitor");
    if (ImPlot::BeginPlot("Job Latency", ImVec2(-1, -1)))
    {
        ImPlot::SetupAxes("Job Index", "Seconds", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        if (!m_LatencyHistory.empty())
        {
            ImPlot::PlotBars("Time", m_LatencyHistory.data(), (int)m_LatencyHistory.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}
