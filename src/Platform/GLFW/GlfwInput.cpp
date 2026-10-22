/// @file GlfwInput.cpp
/// @brief GLFW implementation of the Input polling interface.
/// @details Provides concrete implementations of all Input virtual methods using
///          the GLFW API to query keyboard and mouse state. The static Input::s_Instance
///          is initialized to a GlfwInput at program startup.
/// @ingroup Platform

module;

#include <GLFW/glfw3.h>

module Nodens.Input;

import Nodens.Application;
import Nodens.KeyCodes;
import Nodens.MouseButtonCodes;
import std;

namespace Nodens
{

/// @brief Concrete GLFW-based Input implementation.
/// @details Queries the current GLFW window (obtained via Application::Get()) for
///          key and mouse state using glfwGetKey, glfwGetMouseButton, and glfwGetCursorPos.
/// @ingroup Platform
class GlfwInput : public Input
{
protected:
    /// @brief Checks whether a keyboard key is currently pressed via GLFW.
    /// @param keycode The key to query.
    /// @return True if the key is pressed or held (GLFW_PRESS or GLFW_REPEAT).
    bool IsKeyPressedImpl(EKeyboardKey keycode) override
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        int state{glfwGetKey(window, std::to_underlying(keycode))};
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    /// @brief Checks whether a mouse button is currently pressed via GLFW.
    /// @param buttoncode The mouse button to query.
    /// @return True if the button is pressed (GLFW_PRESS).
    bool IsMouseButtonPressedImpl(EMouseButton buttoncode) override
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        int state{glfwGetMouseButton(window, std::to_underlying(buttoncode))};
        return state == GLFW_PRESS;
    }

    /// @brief Returns the current mouse cursor position via GLFW.
    /// @return A pair of (x, y) coordinates in window-relative pixels.
    std::pair<float, float> GetMousePositionImpl() override
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        return {(float)xPos, (float)yPos};
    }

    /// @brief Returns the current mouse X coordinate.
    /// @return The X position in window-relative pixels.
    float GetMouseXImpl() override
    {
        return GetMousePositionImpl().first;
    }

    /// @brief Returns the current mouse Y coordinate.
    /// @return The Y position in window-relative pixels.
    float GetMouseYImpl() override
    {
        return GetMousePositionImpl().second;
    }
};

/// @brief Static initialization of the Input singleton with the GLFW implementation.
Input* Input::s_Instance = new GlfwInput();

} // namespace Nodens
