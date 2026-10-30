/// @file HeadlessPulseApp.cpp
/// @brief A Nodens application whose main loop runs with no window at all.
/// @details Shows the smallest windowless application: the config asks for no window and no
///          ImGui overlay, a layer counts steps, and the layer ends the application itself
///          through Application::RequestStop(). The run prints a summary of steps, simulated
///          time, wall time and time per step, so `--fixed` and `--variable` can be compared
///          run by run.

#include <chrono>
#include <cstdio>
#include <string_view>

import nodens;

namespace
{
/// @brief Number of steps this example runs before it stops itself.
constexpr int kStepsToRun = 600;

/// @brief Counters the layer fills in, printed once the loop has ended.
struct FPulseStats
{
    int Steps{0};                 ///< Steps the loop ran.
    double SimulatedSeconds{0.0}; ///< Time those steps represent.
};

/// @brief Counts loop steps and stops the application after a fixed number of them.
class PulseLayer : public Nodens::ILayer
{
public:
    /// @param stats The counters to fill as the loop runs.
    explicit PulseLayer(FPulseStats& stats) : ILayer("Pulse"), m_Stats(stats)
    {
    }

    void OnUpdate(Nodens::TimeStep ts) override
    {
        ++m_Stats.Steps;
        m_Stats.SimulatedSeconds += ts.GetSeconds();

        if (m_Stats.Steps % 100 == 0)
            std::printf("[headlesspulse] step %d\n", m_Stats.Steps);

        if (m_Stats.Steps >= kStepsToRun)
            Nodens::Application::Get().RequestStop();
    }

private:
    FPulseStats& m_Stats; ///< Counters owned by main, so the summary outlives the layer.
};
} // namespace

int main(int argc, char** argv)
{
    Nodens::InitializeLoggers();

    Nodens::FApplicationConfig config{};
    config.Name = "HeadlessPulse";
    config.Presentation = Nodens::EPresentation::Headless;

    const char* mode = "variable";
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        if (arg == "--fixed")
        {
            config.Pacing.Pacing = Nodens::EPacing::Fixed;
            mode = "fixed";
        }
        else if (arg == "--variable")
        {
            config.Pacing.Pacing = Nodens::EPacing::Variable;
            mode = "variable";
        }
    }

    FPulseStats stats{};
    const auto started = std::chrono::steady_clock::now();

    Nodens::Application app{config};
    app.PushLayer(new PulseLayer(stats));
    app.Run();

    const double wallSeconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    const double perStep = stats.Steps > 0 ? stats.SimulatedSeconds / stats.Steps : 0.0;

    std::printf(
        "[headlesspulse] mode %s: %d steps, %.3f s simulated, %.3f s wall, %.6f s per step\n",
        mode,
        stats.Steps,
        stats.SimulatedSeconds,
        wallSeconds,
        perStep);

    return 0;
}
