#pragma once
#include "imgui.h"
#include "impedance_viewer_view.h"

#include <algorithm>
#include <cmath>

namespace elda::views::impedance_viewer
{

static inline void center_to_toolbar_y(float toolbar_h, float item_h)
{
    ImGui::SetCursorPosY(std::max(0.0f, (toolbar_h - item_h) * 0.5f));
}

static void render_right_buttons(const ImpedanceViewerViewCallbacks& callbacks, float toolbar_h)
{
    const ImVec4 gray = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    const ImVec4 gray_h = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);
    const ImVec4 blue = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blue_h = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);

    const ImVec2 btn_size(100, 36);
    const float gap = 8.0f;
    const float pad_x = 12.0f;

    const float group_width = btn_size.x * 2.0f + gap;
    const float avail_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

    const float x_right = avail_width - pad_x - group_width;
    const float y_center = (toolbar_h - btn_size.y) * 0.5f;

    ImGui::SetCursorPos(ImVec2(x_right, y_center));
    ImGui::PushStyleColor(ImGuiCol_Button, gray);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, gray_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, gray_h);
    if (ImGui::Button("SETTINGS", btn_size))
    {
        if (callbacks.on_redirect_to_settings)
            callbacks.on_redirect_to_settings();
    }
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPos(ImVec2(x_right + btn_size.x + gap, y_center));
    ImGui::PushStyleColor(ImGuiCol_Button, blue);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blue_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, blue_h);
    if (ImGui::Button("OK", btn_size))
    {
        if (callbacks.on_redirect_to_monitoring)
            callbacks.on_redirect_to_monitoring();
    }
    ImGui::PopStyleColor(3);
}

inline float render_impedance_viewer_toolbar(const ImpedanceViewerViewCallbacks& callbacks)
{
    const float header_h = 52.0f;
    const ImVec4 plot_bg = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, plot_bg);
    ImGui::BeginChild("impedance_header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    render_right_buttons(callbacks, header_h);

    ImGui::EndChild();
    ImGui::PopStyleColor();
    return header_h;
}

}  // namespace elda::views::impedance_viewer
