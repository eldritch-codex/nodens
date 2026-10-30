/// @file FramePacer.cppm
/// @brief Frame timing for the application loop.
/// @ingroup Core

export module Nodens.FramePacer;

import Nodens.TimeStep;
import std;

export namespace Nodens
{

/// @brief How the main loop turns wall time into simulation steps.
/// @ingroup Core
enum class EPacing : std::uint8_t
{
    Variable, ///< One step per iteration, of the measured frame time.
    Fixed,    ///< Steps of a constant length, as many as the elapsed time allows.
};

/// @brief Timing policy for Application::Run().
/// @details The loop asks the pacer for time each iteration instead of reading the window, so a
///          windowless application is paced the same way as a windowed one.
/// @ingroup Core
struct FPacingConfig
{
    EPacing Pacing{EPacing::Variable}; ///< The mode the pacer runs in.

    /// @brief Length of one simulation step in Fixed mode, in seconds.
    float FixedDelta{1.0f / 60.0f};

    /// @brief Upper bound on one frame of wall time, in seconds.
    /// @details A stall beyond this is reported as this value, so a layer never receives a
    ///          multi-second step after the window is dragged or the process is suspended.
    float MaxFrameDelta{0.25f};

    /// @brief Steps one iteration may run in Fixed mode, which caps catch-up work.
    int MaxStepsPerFrame{5};
};

/// @brief The time one iteration of the main loop advances the application by.
/// @ingroup Core
struct FFrameTime
{
    TimeStep Step{0.0f}; ///< One simulation step.
    int Steps{1};        ///< Steps to run this iteration; 0 when a fixed step is not due yet.
};

/// @brief Samples wall time for the main loop, clamps a stalled frame, and paces fixed steps.
/// @details Reads a monotonic clock only. Nothing here touches the window or the graphics
///          backend, which is what lets the loop run without a window.
/// @see Application::Run, FPacingConfig
/// @ingroup Core
class FramePacer
{
public:
    /// @brief Constructs the pacer, starts its clock and validates the policy.
    /// @param config The pacing policy to apply.
    explicit FramePacer(const FPacingConfig& config = FPacingConfig());

    /// @brief Starts one loop iteration.
    /// @details Returns the time since the previous call, clamped to
    ///          FPacingConfig::MaxFrameDelta. In Fixed mode the elapsed time accumulates and
    ///          this returns the steps now due, at most FPacingConfig::MaxStepsPerFrame.
    /// @return The step and step count for this iteration.
    FFrameTime BeginFrame();

    /// @brief Ends one loop iteration.
    /// @details In Fixed mode it sleeps until the next step boundary, so the loop does not spin;
    ///          in Variable mode it does nothing and the window swap paces the loop.
    void EndFrame();

private:
    FPacingConfig m_Config;                            ///< The pacing policy in force.
    std::chrono::steady_clock::time_point m_LastFrame; ///< Timestamp of the previous iteration.
    float m_Accumulator{0.0f};                         ///< Fixed mode: unspent wall time.
};

} // namespace Nodens
