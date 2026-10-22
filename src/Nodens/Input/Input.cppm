/// @file Input.cppm
/// @brief Platform-agnostic singleton interface for polling keyboard and mouse state.
/// @ingroup Input

export module Nodens.Input;

import Nodens.KeyCodes;
import Nodens.MouseButtonCodes;
import std;

export namespace Nodens
{
/// @brief      A singleton-based abstract base class for handling keyboard and mouse input.
///             This class provides a platform-agnostic interface for querying the state of input
///             devices. It is meant to be subclassed and implemented for a specific windowing
///             system or platform API (e.g., GLFW).
class Input
{
public:
    /// @brief      Checks if a specific key is currently pressed.
    /// @param[in]  keycode The key to check.
    /// @return     `true` if the key is pressed, `false` otherwise.
    inline static bool IsKeyPressed(EKeyboardKey keycode)
    {
        return s_Instance->IsKeyPressedImpl(keycode);
    }

    /// @brief      Checks if a specific mouse button is currently pressed.
    /// @param[in]  button The mouse button to check.
    /// @return     `true` if the mouse button is pressed, `false` otherwise.
    inline static bool IsMouseButtonPressed(EMouseButton button)
    {
        return s_Instance->IsMouseButtonPressedImpl(button);
    }

    /// @brief      Gets the current position of the mouse cursor.
    /// @return     A pair of floats representing the X and Y coordinates of the mouse cursor.
    inline static std::pair<float, float> GetMousePosition()
    {
        return s_Instance->GetMousePositionImpl();
    }

    /// @brief      Gets the current X coordinate of the mouse cursor.
    /// @return     The X coordinate of the mouse cursor.
    inline static float GetMouseX()
    {
        return s_Instance->GetMouseXImpl();
    }

    /// @brief      Gets the current Y coordinate of the mouse cursor.
    /// @return     The Y coordinate of the mouse cursor.
    inline static float GetMouseY()
    {
        return s_Instance->GetMouseYImpl();
    }

protected:
    /// @brief      Platform-specific implementation for checking if a key is pressed.
    /// @param[in]  keycode The key to check.
    /// @return     `true` if the key is pressed, `false` otherwise.
    virtual bool IsKeyPressedImpl(EKeyboardKey keycode) = 0;

    /// @brief      Platform-specific implementation for checking if a mouse button is pressed.
    /// @param[in]  button The mouse button to check.
    /// @return     `true` if the mouse button is pressed, `false` otherwise.
    virtual bool IsMouseButtonPressedImpl(EMouseButton button) = 0;

    /// @brief      Platform-specific implementation for getting the mouse position.
    /// @return     A pair of floats representing the X and Y coordinates of the mouse.
    virtual std::pair<float, float> GetMousePositionImpl() = 0;

    /// @brief      Platform-specific implementation for getting the mouse X coordinate.
    /// @return     The X coordinate of the mouse.
    virtual float GetMouseXImpl() = 0;

    /// @brief      Platform-specific implementation for getting the mouse Y coordinate.
    /// @return     The Y coordinate of the mouse.
    virtual float GetMouseYImpl() = 0;

private:
    static Input* s_Instance;
};

} // namespace Nodens
