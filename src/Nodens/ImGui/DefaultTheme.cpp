module;

#include <imgui.h>
#include <implot.h>

/// @file DefaultTheme.cpp
/// @brief Implementation of the default visual themes for Nodens GUI applications.

module Nodens.DefaultTheme;

namespace Nodens
{

// =============================================================================
// Shared Geometry
// =============================================================================
/// @brief Applies geometry settings (rounding, padding, borders, spacing) shared by all Nodens
/// themes.
/// @details This configures the structural look-and-feel of ImGui and ImPlot, independently of
/// colors.
static void ApplySharedGeometry()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.ChildBorderSize = 0.0f;

    style.WindowPadding = {12.0f, 12.0f};
    style.FramePadding = {8.0f, 4.0f};
    style.ItemSpacing = {8.0f, 6.0f};
    style.ItemInnerSpacing = {6.0f, 4.0f};
    style.CellPadding = {6.0f, 4.0f};
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.IndentSpacing = 20.0f;

    style.SeparatorTextBorderSize = 2.0f;

    ImPlotStyle& pstyle = ImPlot::GetStyle();
    pstyle.PlotBorderSize = 0.0f;
    pstyle.MinorAlpha = 0.15f;
    pstyle.PlotPadding = {12, 12};
    pstyle.LabelPadding = {5, 5};
}

// =============================================================================
// Dark Theme
// =============================================================================
/// @brief Internal implementation of the dark color palette.
static void ApplyDarkThemeImpl()
{
    // Backgrounds (layered elevation)
    ImVec4 kBgDarkest = {0.10f, 0.11f, 0.14f, 1.00f}; // #1a1c24
    ImVec4 kBgDark = {0.13f, 0.14f, 0.17f, 1.00f};    // #21242b
    ImVec4 kBgCard = {0.16f, 0.17f, 0.21f, 1.00f};    // #292b36
    ImVec4 kBgPopup = {0.18f, 0.19f, 0.24f, 1.00f};   // #2e303d

    // Borders
    ImVec4 kBorder = {0.24f, 0.25f, 0.30f, 0.60f};      // #3d404c99
    ImVec4 kBorderLight = {0.30f, 0.31f, 0.36f, 0.40f}; // #4c4f5c66

    // Accent (teal) - primary interactive color
    ImVec4 kAccent = {0.24f, 0.79f, 0.65f, 1.00f};      // #3dc9a6
    ImVec4 kAccentHover = {0.30f, 0.86f, 0.72f, 1.00f}; // #4cdbb8
    ImVec4 kAccentMuted = {0.24f, 0.79f, 0.65f, 0.20f}; // #3dc9a633

    // Text
    ImVec4 kTextPrimary = {0.91f, 0.90f, 0.89f, 1.00f}; // #e8e6e3
    ImVec4 kTextMuted = {0.40f, 0.41f, 0.44f, 1.00f};   // #666970

    // Table
    ImVec4 kRowEven = {0.14f, 0.15f, 0.18f, 1.00f};  // #24262e
    ImVec4 kRowOdd = {0.16f, 0.17f, 0.21f, 1.00f};   // #292b36
    ImVec4 kHeaderBg = {0.12f, 0.13f, 0.16f, 1.00f}; // #1f2129

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    // Windows & Panels
    c[ImGuiCol_WindowBg] = kBgDark;
    c[ImGuiCol_ChildBg] = kBgDark;
    c[ImGuiCol_PopupBg] = kBgPopup;
    c[ImGuiCol_Border] = kBorder;
    c[ImGuiCol_BorderShadow] = {0, 0, 0, 0}; // #00000000

    // Frames (inputs, combos, etc.)
    c[ImGuiCol_FrameBg] = kBgCard;
    c[ImGuiCol_FrameBgHovered] = {0.20f, 0.22f, 0.27f, 1.0f}; // #333845
    c[ImGuiCol_FrameBgActive] = {0.22f, 0.24f, 0.30f, 1.0f};  // #383d4c

    // Title bars
    c[ImGuiCol_TitleBg] = kBgDarkest;
    c[ImGuiCol_TitleBgActive] = {0.12f, 0.13f, 0.17f, 1.0f}; // #1f212b
    c[ImGuiCol_TitleBgCollapsed] = kBgDarkest;

    // Menu & Scrollbar
    c[ImGuiCol_MenuBarBg] = kBgDarkest;
    c[ImGuiCol_ScrollbarBg] = {0.10f, 0.11f, 0.13f, 0.5f};          // #1a1c2180
    c[ImGuiCol_ScrollbarGrab] = {0.30f, 0.31f, 0.35f, 1.0f};        // #4c4f59
    c[ImGuiCol_ScrollbarGrabHovered] = {0.38f, 0.39f, 0.43f, 1.0f}; // #61636e
    c[ImGuiCol_ScrollbarGrabActive] = kAccent;

    // Interactive elements
    c[ImGuiCol_CheckMark] = kAccent;
    c[ImGuiCol_SliderGrab] = kAccent;
    c[ImGuiCol_SliderGrabActive] = kAccentHover;

    // Buttons
    c[ImGuiCol_Button] = kBgCard;
    c[ImGuiCol_ButtonHovered] = {0.24f, 0.79f, 0.65f, 0.30f}; // #3dc9a64c
    c[ImGuiCol_ButtonActive] = {0.18f, 0.65f, 0.53f, 1.00f};  // #2ea687

    // Headers (collapsing headers, menu items, selectables)
    c[ImGuiCol_Header] = kAccentMuted;
    c[ImGuiCol_HeaderHovered] = {0.24f, 0.79f, 0.65f, 0.35f}; // #3dc9a659
    c[ImGuiCol_HeaderActive] = {0.24f, 0.79f, 0.65f, 0.45f};  // #3dc9a673

    // Separators
    c[ImGuiCol_Separator] = kBorder;
    c[ImGuiCol_SeparatorHovered] = kAccent;
    c[ImGuiCol_SeparatorActive] = kAccentHover;

    // Resize grips
    c[ImGuiCol_ResizeGrip] = {0.24f, 0.79f, 0.65f, 0.15f};        // #3dc9a626
    c[ImGuiCol_ResizeGripHovered] = {0.24f, 0.79f, 0.65f, 0.40f}; // #3dc9a666
    c[ImGuiCol_ResizeGripActive] = kAccent;

    // Tabs
    c[ImGuiCol_Tab] = kBgCard;
    c[ImGuiCol_TabHovered] = {0.24f, 0.79f, 0.65f, 0.35f};  // #3dc9a659
    c[ImGuiCol_TabSelected] = {0.24f, 0.79f, 0.65f, 0.25f}; // #3dc9a640
    c[ImGuiCol_TabDimmed] = kBgDarkest;
    c[ImGuiCol_TabDimmedSelected] = kBgCard;

    // Docking
    c[ImGuiCol_DockingPreview] = {0.24f, 0.79f, 0.65f, 0.50f}; // #3dc9a680
    c[ImGuiCol_DockingEmptyBg] = kBgDarkest;

    // Plots (ImGui built-in, not ImPlot)
    c[ImGuiCol_PlotLines] = kAccent;
    c[ImGuiCol_PlotLinesHovered] = kAccentHover;
    c[ImGuiCol_PlotHistogram] = kAccent;
    c[ImGuiCol_PlotHistogramHovered] = kAccentHover;

    // Tables
    c[ImGuiCol_TableHeaderBg] = kHeaderBg;
    c[ImGuiCol_TableBorderStrong] = kBorder;
    c[ImGuiCol_TableBorderLight] = kBorderLight;
    c[ImGuiCol_TableRowBg] = kRowEven;
    c[ImGuiCol_TableRowBgAlt] = kRowOdd;

    // Misc
    c[ImGuiCol_TextSelectedBg] = {0.24f, 0.79f, 0.65f, 0.25f}; // #3dc9a640
    c[ImGuiCol_DragDropTarget] = kAccent;
    c[ImGuiCol_NavHighlight] = kAccent;

    // Text
    c[ImGuiCol_Text] = kTextPrimary;
    c[ImGuiCol_TextDisabled] = kTextMuted;

    // ImPlot Theme
    ImPlotStyle& pstyle = ImPlot::GetStyle();
    static const ImVec4 kPalette[] = {
        {0.24f, 0.79f, 0.65f, 1.0f}, // teal      #3dc9a6
        {0.95f, 0.75f, 0.25f, 1.0f}, // amber     #f2bf40
        {0.90f, 0.45f, 0.40f, 1.0f}, // coral     #e67366
        {0.65f, 0.55f, 0.90f, 1.0f}, // lavender  #a68ce6
        {0.40f, 0.72f, 0.96f, 1.0f}, // sky blue  #66b8f5
        {0.55f, 0.88f, 0.35f, 1.0f}, // lime      #8ce059
        {0.90f, 0.50f, 0.70f, 1.0f}, // rose      #e680b2
    };
    static ImPlotColormap customMap = ImPlot::AddColormap("NodensDark", kPalette, 7);
    ImPlot::PushColormap(customMap);

    ImVec4* pc = pstyle.Colors;
    pc[ImPlotCol_PlotBg] = {0.11f, 0.12f, 0.15f, 1.0f};      // #1c1f26
    pc[ImPlotCol_PlotBorder] = {0.22f, 0.23f, 0.28f, 0.5f};  // #383b4780
    pc[ImPlotCol_FrameBg] = {0.0f, 0.0f, 0.0f, 0.0f};        // #00000000
    pc[ImPlotCol_AxisGrid] = {0.25f, 0.26f, 0.30f, 0.30f};   // #40424c4c
    pc[ImPlotCol_AxisText] = {0.55f, 0.56f, 0.58f, 1.00f};   // #8c8f94
    pc[ImPlotCol_LegendBg] = {0.14f, 0.15f, 0.19f, 0.90f};   // #242630e6
    pc[ImPlotCol_LegendText] = {0.91f, 0.90f, 0.89f, 1.00f}; // #e8e6e3
}

// =============================================================================
// Light Theme
// =============================================================================
/// @brief Internal implementation of the light color palette.
static void ApplyLightThemeImpl()
{
    // Backgrounds
    ImVec4 kBgLightest = {1.00f, 1.00f, 1.00f, 1.00f}; // #ffffff
    ImVec4 kBgLight = {0.97f, 0.97f, 0.98f, 1.00f};    // #f7f7fa
    ImVec4 kBgCard = {0.93f, 0.93f, 0.94f, 1.00f};     // #ededf0
    ImVec4 kBgPopup = {1.00f, 1.00f, 1.00f, 1.00f};    // #ffffff

    // Borders
    ImVec4 kBorder = {0.80f, 0.80f, 0.83f, 1.00f};      // #ccccd4
    ImVec4 kBorderLight = {0.85f, 0.85f, 0.88f, 1.00f}; // #d9d9e0

    // Accent (deep teal for light mode)
    ImVec4 kAccent = {0.10f, 0.60f, 0.50f, 1.00f};      // #1a9980
    ImVec4 kAccentHover = {0.15f, 0.70f, 0.60f, 1.00f}; // #26b299
    ImVec4 kAccentMuted = {0.10f, 0.60f, 0.50f, 0.20f}; // #1a998033

    // Text
    ImVec4 kTextPrimary = {0.13f, 0.14f, 0.17f, 1.00f}; // #21242b
    ImVec4 kTextMuted = {0.40f, 0.41f, 0.44f, 1.00f};   // #666970

    // Table
    ImVec4 kRowEven = {1.00f, 1.00f, 1.00f, 1.00f};  // #ffffff
    ImVec4 kRowOdd = {0.97f, 0.97f, 0.98f, 1.00f};   // #f7f7fa
    ImVec4 kHeaderBg = {0.93f, 0.93f, 0.94f, 1.00f}; // #ededf0

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    // Windows & Panels
    c[ImGuiCol_WindowBg] = kBgLight;
    c[ImGuiCol_ChildBg] = kBgLight;
    c[ImGuiCol_PopupBg] = kBgPopup;
    c[ImGuiCol_Border] = kBorder;
    c[ImGuiCol_BorderShadow] = {0, 0, 0, 0}; // #00000000

    // Frames (inputs, combos, etc.)
    c[ImGuiCol_FrameBg] = kBgCard;
    c[ImGuiCol_FrameBgHovered] = {0.85f, 0.85f, 0.88f, 1.0f}; // #d9d9e0
    c[ImGuiCol_FrameBgActive] = {0.80f, 0.80f, 0.83f, 1.0f};  // #ccccd4

    // Title bars
    c[ImGuiCol_TitleBg] = kBgLightest;
    c[ImGuiCol_TitleBgActive] = {0.90f, 0.90f, 0.92f, 1.0f}; // #e6e6eb
    c[ImGuiCol_TitleBgCollapsed] = kBgLightest;

    // Menu & Scrollbar
    c[ImGuiCol_MenuBarBg] = kBgLightest;
    c[ImGuiCol_ScrollbarBg] = {0.97f, 0.97f, 0.98f, 0.5f};          // #f7f7fa80
    c[ImGuiCol_ScrollbarGrab] = {0.80f, 0.80f, 0.83f, 1.0f};        // #ccccd4
    c[ImGuiCol_ScrollbarGrabHovered] = {0.70f, 0.70f, 0.73f, 1.0f}; // #b2b2ba
    c[ImGuiCol_ScrollbarGrabActive] = kAccent;

    // Interactive elements
    c[ImGuiCol_CheckMark] = kAccent;
    c[ImGuiCol_SliderGrab] = kAccent;
    c[ImGuiCol_SliderGrabActive] = kAccentHover;

    // Buttons
    c[ImGuiCol_Button] = kBgCard;
    c[ImGuiCol_ButtonHovered] = {0.10f, 0.60f, 0.50f, 0.30f}; // #1a99804c
    c[ImGuiCol_ButtonActive] = {0.05f, 0.50f, 0.40f, 1.00f};  // #0d8066

    // Headers (collapsing headers, menu items, selectables)
    c[ImGuiCol_Header] = kAccentMuted;
    c[ImGuiCol_HeaderHovered] = {0.10f, 0.60f, 0.50f, 0.35f}; // #1a998059
    c[ImGuiCol_HeaderActive] = {0.10f, 0.60f, 0.50f, 0.45f};  // #1a998073

    // Separators
    c[ImGuiCol_Separator] = kBorder;
    c[ImGuiCol_SeparatorHovered] = kAccent;
    c[ImGuiCol_SeparatorActive] = kAccentHover;

    // Resize grips
    c[ImGuiCol_ResizeGrip] = {0.10f, 0.60f, 0.50f, 0.15f};        // #1a998026
    c[ImGuiCol_ResizeGripHovered] = {0.10f, 0.60f, 0.50f, 0.40f}; // #1a998066
    c[ImGuiCol_ResizeGripActive] = kAccent;

    // Tabs
    c[ImGuiCol_Tab] = kBgCard;
    c[ImGuiCol_TabHovered] = {0.10f, 0.60f, 0.50f, 0.35f};  // #1a998059
    c[ImGuiCol_TabSelected] = {0.10f, 0.60f, 0.50f, 0.25f}; // #1a998040
    c[ImGuiCol_TabDimmed] = kBgLightest;
    c[ImGuiCol_TabDimmedSelected] = kBgCard;

    // Docking
    c[ImGuiCol_DockingPreview] = {0.10f, 0.60f, 0.50f, 0.50f}; // #1a998080
    c[ImGuiCol_DockingEmptyBg] = kBgLightest;

    // Plots (ImGui built-in, not ImPlot)
    c[ImGuiCol_PlotLines] = kAccent;
    c[ImGuiCol_PlotLinesHovered] = kAccentHover;
    c[ImGuiCol_PlotHistogram] = kAccent;
    c[ImGuiCol_PlotHistogramHovered] = kAccentHover;

    // Tables
    c[ImGuiCol_TableHeaderBg] = kHeaderBg;
    c[ImGuiCol_TableBorderStrong] = kBorder;
    c[ImGuiCol_TableBorderLight] = kBorderLight;
    c[ImGuiCol_TableRowBg] = kRowEven;
    c[ImGuiCol_TableRowBgAlt] = kRowOdd;

    // Misc
    c[ImGuiCol_TextSelectedBg] = {0.10f, 0.60f, 0.50f, 0.25f}; // #1a998040
    c[ImGuiCol_DragDropTarget] = kAccent;
    c[ImGuiCol_NavHighlight] = kAccent;

    // Text
    c[ImGuiCol_Text] = kTextPrimary;
    c[ImGuiCol_TextDisabled] = kTextMuted;

    // ImPlot Theme
    ImPlotStyle& pstyle = ImPlot::GetStyle();
    static const ImVec4 kPalette[] = {
        {0.10f, 0.60f, 0.50f, 1.0f}, // deep teal  #1a9980
        {0.80f, 0.60f, 0.10f, 1.0f}, // amber      #cc991a
        {0.80f, 0.30f, 0.25f, 1.0f}, // coral      #cc4c40
        {0.50f, 0.40f, 0.80f, 1.0f}, // lavender   #8066cc
        {0.20f, 0.50f, 0.85f, 1.0f}, // sky blue   #3380d9
        {0.35f, 0.70f, 0.15f, 1.0f}, // lime       #59b226
        {0.80f, 0.30f, 0.60f, 1.0f}, // rose       #cc4c99
    };
    static ImPlotColormap customMap = ImPlot::AddColormap("NodensLight", kPalette, 7);
    ImPlot::PushColormap(customMap);

    ImVec4* pc = pstyle.Colors;
    pc[ImPlotCol_PlotBg] = {0.97f, 0.97f, 0.98f, 1.0f};      // #f7f7fa
    pc[ImPlotCol_PlotBorder] = {0.80f, 0.80f, 0.83f, 1.0f};  // #ccccd4
    pc[ImPlotCol_FrameBg] = {0.0f, 0.0f, 0.0f, 0.0f};        // #00000000
    pc[ImPlotCol_AxisGrid] = {0.80f, 0.80f, 0.83f, 1.00f};   // #ccccd4
    pc[ImPlotCol_AxisText] = {0.40f, 0.41f, 0.44f, 1.00f};   // #666970
    pc[ImPlotCol_LegendBg] = {0.93f, 0.93f, 0.94f, 0.90f};   // #ededf0e6
    pc[ImPlotCol_LegendText] = {0.13f, 0.14f, 0.17f, 1.00f}; // #21242b
}

// =============================================================================
// Public API
// =============================================================================

void ApplyDefaultDarkTheme()
{
    ApplySharedGeometry();
    ApplyDarkThemeImpl();
}

void ApplyDefaultLightTheme()
{
    ApplySharedGeometry();
    ApplyLightThemeImpl();
}

} // namespace Nodens
