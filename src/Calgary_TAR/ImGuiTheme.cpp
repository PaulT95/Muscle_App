#include "Calgary_TAR/ImGuiTheme.hpp"
#include <implot.h>
#include <iostream>
#include <fstream> // For file existence check
// Set a white/light theme for ImGui

namespace ImGuiTheme {

    //set font only ONCE (roboto)
    void setFont()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false; // THIS FIXES THE fucking font crash..what a pain importing a ftt

        ImFont* font = io.Fonts->AddFontFromMemoryTTF(
            (void*)roboto_font_data,
            sizeof(roboto_font_data),
            16.5f,
            &font_cfg
        );

        if (font == nullptr)
        {
            std::cerr << "Failed to load font from memory!" << std::endl;
        }

        io.FontDefault = font;
    }

void ApplyTheme() {
    // if (mode == ThemeMode::Light) {
    //     ApplyLightModern();
    // } else {
    //     ApplyDarkModern();
    // }
    ApplyDarkModern();
}

void ApplyDarkModern() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.52f, 0.54f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.28f, 0.29f, 0.32f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.25f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.18f, 0.20f, 0.23f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.25f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.30f, 0.34f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.35f, 0.40f, 0.48f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.44f, 0.68f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.32f, 0.52f, 0.78f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.44f, 0.68f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.32f, 0.52f, 0.78f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.28f, 0.29f, 0.32f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.28f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.17f, 0.18f, 0.21f, 1.00f);

    style->WindowPadding = ImVec2(8.0f, 8.0f);
    style->FramePadding = ImVec2(6.0f, 3.0f);
    style->CellPadding = ImVec2(6.0f, 5.0f);
    style->ItemSpacing = ImVec2(6.0f, 6.0f);
    style->WindowRounding = 6.0f;
    style->ChildRounding = 5.0f;
    style->FrameRounding = 4.0f;
    style->PopupRounding = 4.0f;
    style->ScrollbarRounding = 6.0f;
    style->GrabRounding = 4.0f;
    style->TabRounding = 4.0f;

    ImPlotStyle& s = ImPlot::GetStyle();
    s.Colors[ImPlotCol_Line]       = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
    s.Colors[ImPlotCol_PlotBg]     = colors[ImGuiCol_ChildBg];
    s.Colors[ImPlotCol_FrameBg]    = colors[ImGuiCol_FrameBg];
    s.Colors[ImPlotCol_PlotBorder] = colors[ImGuiCol_Border];
    s.LineWeight = 2.0f;
}

void ApplyLightModern() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    // Crisp, low-fatigue slate-on-off-white palette matching the dark modern tone
    colors[ImGuiCol_Text]                  = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.55f, 0.58f, 0.62f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.95f, 0.97f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.98f, 0.99f, 1.00f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.92f, 0.93f, 0.96f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.78f, 0.80f, 0.84f, 0.80f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.88f, 0.90f, 0.94f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.82f, 0.85f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.75f, 0.80f, 0.87f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.94f, 0.95f, 0.97f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.94f, 0.95f, 0.97f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.90f, 0.91f, 0.94f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.75f, 0.78f, 0.83f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.65f, 0.69f, 0.75f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.55f, 0.60f, 0.68f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.12f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.18f, 0.52f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.12f, 0.45f, 0.80f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.82f, 0.86f, 0.92f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.74f, 0.80f, 0.88f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.65f, 0.74f, 0.84f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.82f, 0.86f, 0.92f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.74f, 0.80f, 0.88f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.65f, 0.74f, 0.84f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.78f, 0.80f, 0.84f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.88f, 0.90f, 0.94f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.75f, 0.78f, 0.83f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.84f, 0.87f, 0.91f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.98f, 0.99f, 1.00f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.94f, 0.96f, 0.98f, 1.00f);

    style->WindowPadding = ImVec2(8.0f, 8.0f);
    style->FramePadding = ImVec2(6.0f, 3.0f);
    style->CellPadding = ImVec2(6.0f, 5.0f);
    style->ItemSpacing = ImVec2(6.0f, 6.0f);
    style->WindowRounding = 6.0f;
    style->ChildRounding = 5.0f;
    style->FrameRounding = 4.0f;
    style->PopupRounding = 4.0f;
    style->ScrollbarRounding = 6.0f;
    style->GrabRounding = 4.0f;
    style->TabRounding = 4.0f;

    ImPlotStyle& s = ImPlot::GetStyle();
    s.Colors[ImPlotCol_Line]       = ImVec4(0.12f, 0.48f, 0.82f, 1.00f);
    s.Colors[ImPlotCol_PlotBg]     = colors[ImGuiCol_ChildBg];
    s.Colors[ImPlotCol_FrameBg]    = colors[ImGuiCol_FrameBg];
    s.Colors[ImPlotCol_PlotBorder] = colors[ImGuiCol_Border];
    s.LineWeight = 2.0f;
}

} // namespace ImGuiTheme