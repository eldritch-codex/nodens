/// @file FramePacer.cpp
/// @brief Implementation of FramePacer.
/// @ingroup Core

module;

#include <tracy/Tracy.hpp>

module Nodens.FramePacer;

import Nodens.Log;
import std;

namespace Nodens
{

namespace
{
/// @brief Reduces a measured frame time to the configured maximum.
float ClampFrameDelta(float delta, float maxDelta)
{
    return delta > maxDelta ? maxDelta : delta;
}
} // namespace

FramePacer::FramePacer(const FPacingConfig& config)
    : m_Config(config), m_LastFrame(std::chrono::steady_clock::now())
{
    ZoneScoped;

    if (m_Config.MaxFrameDelta <= 0.0f)
        FatalCore("MaxFrameDelta must be positive!");
    if (m_Config.MaxStepsPerFrame < 1)
        FatalCore("MaxStepsPerFrame must be at least 1!");
    if (m_Config.Pacing == EPacing::Fixed && m_Config.FixedDelta <= 0.0f)
        FatalCore("FixedDelta must be positive in fixed pacing!");
}

FFrameTime FramePacer::BeginFrame()
{
    ZoneScoped;

    const auto now = std::chrono::steady_clock::now();
    const float delta = ClampFrameDelta(std::chrono::duration<float>(now - m_LastFrame).count(),
                                        m_Config.MaxFrameDelta);
    m_LastFrame = now;

    if (m_Config.Pacing == EPacing::Variable)
        return FFrameTime{.Step = TimeStep{delta}, .Steps = 1};

    m_Accumulator += delta;
    int steps = static_cast<int>(m_Accumulator / m_Config.FixedDelta);
    if (steps > m_Config.MaxStepsPerFrame)
    {
        // Drop the backlog instead of replaying it, so a long stall cannot stall the loop again.
        steps = m_Config.MaxStepsPerFrame;
        m_Accumulator = 0.0f;
    }
    else
    {
        m_Accumulator -= static_cast<float>(steps) * m_Config.FixedDelta;
    }

    return FFrameTime{.Step = TimeStep{m_Config.FixedDelta}, .Steps = steps};
}

void FramePacer::EndFrame()
{
    ZoneScoped;

    if (m_Config.Pacing != EPacing::Fixed)
        return;

    const auto stepDuration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<float>(m_Config.FixedDelta));
    std::this_thread::sleep_until(m_LastFrame + stepDuration);
}

} // namespace Nodens
