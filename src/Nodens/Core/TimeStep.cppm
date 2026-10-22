/// @file TimeStep.cppm
/// @brief Lightweight wrapper representing a frame delta-time in seconds.
/// @ingroup Core

export module Nodens.TimeStep;

export namespace Nodens
{
/// @brief A thin value type that wraps a frame delta-time measured in seconds.
/// @details TimeStep is implicitly convertible to `float`, making it easy to use
///          directly in arithmetic expressions. It is passed to Layer::OnUpdate()
///          and Layer::OnImGuiRender() every frame.
/// @ingroup Core
class TimeStep
{
public:
    /// @brief Constructs a TimeStep with the given time in seconds.
    /// @param time Elapsed time in seconds (defaults to 0).
    TimeStep(float time = 0.0f) : m_Time(time)
    {
    }

    /// @brief Implicit conversion to float (seconds).
    /// @return The time value in seconds.
    operator float() const
    {
        return m_Time;
    }

    /// @brief Adds a scalar value to the stored time.
    /// @param a The value to add (seconds).
    /// @return The sum as a float.
    float operator+(const float a) const
    {
        return m_Time + a;
    }

    /// @brief Returns the time value in seconds.
    /// @return Seconds as a float.
    float GetSeconds() const
    {
        return m_Time;
    }

    /// @brief Returns the time value converted to milliseconds.
    /// @return Milliseconds as a float.
    float GetMilliseconds() const
    {
        return m_Time * 1000;
    }

private:
    float m_Time; ///< Stored delta-time in seconds.
};
} // namespace Nodens
