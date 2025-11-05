#pragma once
#include "imgui.h"
#include "implot.h"

// High-contrast dark theme tuned for scope visuals (cyan traces, muted grid)
inline void ApplyAldaTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    s.WindowRounding   = 6.0f;
    s.FrameRounding    = 6.0f;
    s.GrabRounding     = 6.0f;
    s.WindowBorderSize = 0.0f;
    s.FrameBorderSize  = 0.0f;
    s.ItemSpacing      = ImVec2(10, 8);
    s.FramePadding     = ImVec2(12, 8);
    s.WindowPadding    = ImVec2(12, 10);

    const ImVec4 bg0     = ImVec4(0.08f, 0.09f, 0.10f, 1.00f);
    const ImVec4 bg1     = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
    const ImVec4 bg2     = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
    const ImVec4 text    = ImVec4(0.85f, 0.87f, 0.90f, 1.00f);
    const ImVec4 textDim = ImVec4(0.65f, 0.68f, 0.72f, 1.00f);
    const ImVec4 sep     = ImVec4(1,1,1,0.08f);

    c[ImGuiCol_Text]            = text;
    c[ImGuiCol_TextDisabled]    = textDim;
    c[ImGuiCol_WindowBg]        = bg0;
    c[ImGuiCol_ChildBg]         = bg0;
    c[ImGuiCol_PopupBg]         = bg1;
    c[ImGuiCol_Border]          = sep;
    c[ImGuiCol_BorderShadow]    = ImVec4(0,0,0,0);
    c[ImGuiCol_FrameBg]         = bg1;
    c[ImGuiCol_FrameBgHovered]  = bg2;
    c[ImGuiCol_FrameBgActive]   = bg2;
    c[ImGuiCol_Button]          = bg1;
    c[ImGuiCol_ButtonHovered]   = bg2;
    c[ImGuiCol_ButtonActive]    = bg2;
    c[ImGuiCol_Header]          = bg1;
    c[ImGuiCol_HeaderHovered]   = bg2;
    c[ImGuiCol_HeaderActive]    = bg2;
    c[ImGuiCol_SliderGrab]      = ImVec4(0.70f,0.75f,0.80f,1.0f);
    c[ImGuiCol_SliderGrabActive]= ImVec4(0.80f,0.85f,0.90f,1.0f);
    c[ImGuiCol_Separator]       = sep;
    c[ImGuiCol_MenuBarBg]       = bg0;

    // ---- ImPlot (enums compatible with your version) ----
    ImPlotStyle& ps = ImPlot::GetStyle();
    ps.LineWeight       = 1.0f;
    ps.PlotBorderSize   = 0.0f;
    ps.MinorAlpha       = 0.25f;

    ImVec4* pc = ps.Colors;
    const ImVec4 plotBg    = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);
    const ImVec4 plotBorder= ImVec4(1,1,1,0.06f);
    const ImVec4 gridMajor = ImVec4(1,1,1,0.08f);
    const ImVec4 gridMinor = ImVec4(1,1,1,0.04f);
    const ImVec4 axisTxt   = textDim;
    const ImVec4 axisTick  = ImVec4(1,1,1,0.06f);
    const ImVec4 cyanTrace = ImVec4(0.30f, 0.90f, 0.95f, 1.00f);

    pc[ImPlotCol_PlotBg]        = plotBg;
    pc[ImPlotCol_PlotBorder]    = plotBorder;

    // Older ImPlot uses these generic axis slots:
    pc[ImPlotCol_AxisText]      = axisTxt;
    pc[ImPlotCol_AxisGrid]      = gridMajor;   // major grid
    pc[ImPlotCol_AxisTick]      = axisTick;    // tick marks
    pc[ImPlotCol_AxisBg]        = ImVec4(0,0,0,0);
    pc[ImPlotCol_AxisBgHovered] = ImVec4(0,0,0,0);
    // (No AxisLine in this version)

    pc[ImPlotCol_Line]          = cyanTrace;
    pc[ImPlotCol_Fill]          = cyanTrace;
}
