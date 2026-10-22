export module Nodens.DefaultTheme;

/// @file DefaultTheme.cppm
/// @brief Default visual theme for all Nodens GUI applications.
///
/// Nodens offers three default visual themes that can be applied to the GUI:
/// - Dark: A professional dark mode theme featuring teal accents and layered elevation.
/// - Light: A bright, clean theme featuring deep teal accents and subtle contrast.
/// - None: Opt out of Nodens styling entirely, falling back to ImGui's default style.

export namespace Nodens
{

enum class EDefaultTheme
{
    None, ///< Do not apply any Nodens styling (fallback to ImGui defaults).
    Dark, ///< The dark mode professional theme with teal accents.
    Light ///< The bright professional theme with deep teal accents.
};

/// @brief Applies the full Nodens dark theme to the current ImGui and ImPlot contexts.
/// This configures both the shared UI geometry and the dark color palette.
void ApplyDefaultDarkTheme();

/// @brief Applies the full Nodens light theme to the current ImGui and ImPlot contexts.
/// This configures both the shared UI geometry and the light color palette.
void ApplyDefaultLightTheme();

} // namespace Nodens
