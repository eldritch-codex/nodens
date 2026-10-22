export module Example.CircularWave3DLayer;

import nodens;
import std;

// -----------------------------------------------------------------------------
// Step 1: Define the Layer class. This layer will be responsible for
// managing the state of the circular wave and rendering it using ImPlot
// and the Nodens GUI infrastructure.
// -----------------------------------------------------------------------------

export class CircularWave3DLayer : public Nodens::ILayer
{
public:
    CircularWave3DLayer();

    void OnUpdate(Nodens::TimeStep ts) override;
    void OnImGuiRender(Nodens::TimeStep ts) override;

private:
    static constexpr std::size_t kNumberOfPoints{1000};

    float m_Frequency = 5.f * std::numbers::pi_v<float>;
    float t[kNumberOfPoints], xs[kNumberOfPoints], ys[kNumberOfPoints];
};
