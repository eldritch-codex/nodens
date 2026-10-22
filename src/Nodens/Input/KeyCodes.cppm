/// @file KeyCodes.cppm
/// @brief Keyboard key code enumeration (GLFW-based) and stream operator.
/// @ingroup Input

export module Nodens.KeyCodes;

import std;

export namespace Nodens
{
/// @brief      Key codes for keyboard keys.
///             These key codes are based on the key codes from glfw3.h.
enum class EKeyboardKey : std::uint16_t
{
    // From glfw3.h
    Space = 32,      ///< The Space key.
    Apostrophe = 39, ///< The ' key.
    Comma = 44,      ///< The , key.
    Minus = 45,      ///< The - key.
    Period = 46,     ///< The . key.
    Slash = 47,      ///< The / key.

    D0 = 48, ///< The 0 key.
    D1 = 49, ///< The 1 key.
    D2 = 50, ///< The 2 key.
    D3 = 51, ///< The 3 key.
    D4 = 52, ///< The 4 key.
    D5 = 53, ///< The 5 key.
    D6 = 54, ///< The 6 key.
    D7 = 55, ///< The 7 key.
    D8 = 56, ///< The 8 key.
    D9 = 57, ///< The 9 key.

    Semicolon = 59, ///< The ; key.
    Equal = 61,     ///< The = key.

    A = 65, ///< The A key.
    B = 66, ///< The B key.
    C = 67, ///< The C key.
    D = 68, ///< The D key.
    E = 69, ///< The E key.
    F = 70, ///< The F key.
    G = 71, ///< The G key.
    H = 72, ///< The H key.
    I = 73, ///< The I key.
    J = 74, ///< The J key.
    K = 75, ///< The K key.
    L = 76, ///< The L key.
    M = 77, ///< The M key.
    N = 78, ///< The N key.
    O = 79, ///< The O key.
    P = 80, ///< The P key.
    Q = 81, ///< The Q key.
    R = 82, ///< The R key.
    S = 83, ///< The S key.
    T = 84, ///< The T key.
    U = 85, ///< The U key.
    V = 86, ///< The V key.
    W = 87, ///< The W key.
    X = 88, ///< The X key.
    Y = 89, ///< The Y key.
    Z = 90, ///< The Z key.

    LeftBracket = 91,  ///< The [ key.
    Backslash = 92,    ///< The \ key.
    RightBracket = 93, ///< The ] key.
    GraveAccent = 96,  ///< The ` key.

    World1 = 161, ///< The World 1 key (non-US #1).
    World2 = 162, ///< The World 2 key (non-US #2).

    /// Function keys
    Escape = 256,      ///< The Escape key.
    Enter = 257,       ///< The Enter key.
    Tab = 258,         ///< The Tab key.
    Backspace = 259,   ///< The Backspace key.
    Insert = 260,      ///< The Insert key.
    Delete = 261,      ///< The Delete key.
    Right = 262,       ///< The Right Arrow key.
    Left = 263,        ///< The Left Arrow key.
    Down = 264,        ///< The Down Arrow key.
    Up = 265,          ///< The Up Arrow key.
    PageUp = 266,      ///< The Page Up key.
    PageDown = 267,    ///< The Page Down key.
    Home = 268,        ///< The Home key.
    End = 269,         ///< The End key.
    CapsLock = 280,    ///< The Caps Lock key.
    ScrollLock = 281,  ///< The Scroll Lock key.
    NumLock = 282,     ///< The Num Lock key.
    PrintScreen = 283, ///< The Print Screen key.
    Pause = 284,       ///< The Pause key.
    F1 = 290,          ///< The F1 key.
    F2 = 291,          ///< The F2 key.
    F3 = 292,          ///< The F3 key.
    F4 = 293,          ///< The F4 key.
    F5 = 294,          ///< The F5 key.
    F6 = 295,          ///< The F6 key.
    F7 = 296,          ///< The F7 key.
    F8 = 297,          ///< The F8 key.
    F9 = 298,          ///< The F9 key.
    F10 = 299,         ///< The F10 key.
    F11 = 300,         ///< The F11 key.
    F12 = 301,         ///< The F12 key.
    F13 = 302,         ///< The F13 key.
    F14 = 303,         ///< The F14 key.
    F15 = 304,         ///< The F15 key.
    F16 = 305,         ///< The F16 key.
    F17 = 306,         ///< The F17 key.
    F18 = 307,         ///< The F18 key.
    F19 = 308,         ///< The F19 key.
    F20 = 309,         ///< The F20 key.
    F21 = 310,         ///< The F21 key.
    F22 = 311,         ///< The F22 key.
    F23 = 312,         ///< The F23 key.
    F24 = 313,         ///< The F24 key.
    F25 = 314,         ///< The F25 key.

    /// Keypad
    KP0 = 320,        ///< The Keypad 0 key.
    KP1 = 321,        ///< The Keypad 1 key.
    KP2 = 322,        ///< The Keypad 2 key.
    KP3 = 323,        ///< The Keypad 3 key.
    KP4 = 324,        ///< The Keypad 4 key.
    KP5 = 325,        ///< The Keypad 5 key.
    KP6 = 326,        ///< The Keypad 6 key.
    KP7 = 327,        ///< The Keypad 7 key.
    KP8 = 328,        ///< The Keypad 8 key.
    KP9 = 329,        ///< The Keypad 9 key.
    KPDecimal = 330,  ///< The Keypad . key.
    KPDivide = 331,   ///< The Keypad / key.
    KPMultiply = 332, ///< The Keypad * key.
    KPSubtract = 333, ///< The Keypad - key.
    KPAdd = 334,      ///< The Keypad + key.
    KPEnter = 335,    ///< The Keypad Enter key.
    KPEqual = 336,    ///< The Keypad = key.

    LeftShift = 340,    ///< The Left Shift key.
    LeftControl = 341,  ///< The Left Control key.
    LeftAlt = 342,      ///< The Left Alt key.
    LeftSuper = 343,    ///< The Left Super key.
    RightShift = 344,   ///< The Right Shift key.
    RightControl = 345, ///< The Right Control key.
    RightAlt = 346,     ///< The Right Alt key.
    RightSuper = 347,   ///< The Right Super key.
    Menu = 348          ///< The Menu key.
};

/// @brief      Overload of the << operator for the Key enum.
///             This allows printing Key codes directly to output streams (e.g., for logging).
/// @param      os  The output stream.
/// @param[in]  key The key code to print.
/// @return     The output stream.
inline std::ostream& operator<<(std::ostream& os, EKeyboardKey key)
{
    os << std::to_underlying(key);
    return os;
}
} // namespace Nodens
