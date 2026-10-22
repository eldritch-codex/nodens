export module Example.PlanetaryScanLayer;
import nodens;
import std;

// -----------------------------------------------------------------------------
// Step 1: Define the Event class. This event will be used to communicate
// the results of the async task to the main thread.
// -----------------------------------------------------------------------------

export struct PlanetaryScanResultEvent
{
    static constexpr std::string_view Name = "PlanetaryScanResult";
    int m_ID{};
    float m_Distance{};
    float m_AtmosphereDensity{};
    float m_CalculationTime{};
};

// -----------------------------------------------------------------------------
// Optional: a std::formatter so the event can be logged or std::format'd.
// Declared in the same module as the event, right after it.
// -----------------------------------------------------------------------------

template <>
struct std::formatter<PlanetaryScanResultEvent> : std::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const PlanetaryScanResultEvent& e, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(),
                              "PlanetaryScanResult(id={}, distance={:.2f} ly, density={:.3f}, "
                              "latency={:.1f} ms)",
                              e.m_ID,
                              e.m_Distance,
                              e.m_AtmosphereDensity,
                              e.m_CalculationTime * 1000.0f);
    }
};

// -----------------------------------------------------------------------------
// Step 2: Define the Layer class. This layer will be responsible for
// creating async tasks, handling their results, and rendering the UI.
// It is essentially the heart of the application, managing all the logic
// and data.
// -----------------------------------------------------------------------------

export class PlanetaryScanLayer : public Nodens::ILayer
{
public:
    PlanetaryScanLayer();
    ~PlanetaryScanLayer() override = default;

    void OnAttach() override;
    void OnUpdate(Nodens::TimeStep ts) override;
    void OnImGuiRender(Nodens::TimeStep ts) override;

private:
    void AddResult(const PlanetaryScanResultEvent& e);

private:
    float m_TimePassed{};
    int m_ScanCounter{};

    std::mutex m_DataMutex;

    std::vector<float> m_GalaxyDistances{};
    std::vector<float> m_GalaxyDensities{};

    std::vector<float> m_LatencyHistory{};
    float m_MaxLatency{};
};
