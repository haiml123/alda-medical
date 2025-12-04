#pragma once

#include "imgui.h"

#include <algorithm>
#include <functional>

namespace elda::views::user_settings
{

struct UserSettingsHeaderCallbacks
{
    std::function<void()> on_proceed;
    std::function<void()> on_back;
};

struct UserSettingsHeaderData
{
    bool can_proceed = false;
};

inline float render_user_settings_header(const UserSettingsHeaderData& data,
                                         const UserSettingsHeaderCallbacks& callbacks)
{
    const float header_h = 52.0f;
    const ImVec4 header_bg = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

    const ImVec4 blue = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blue_h = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 gray = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    const ImVec4 gray_h = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, header_bg);
    ImGui::BeginChild("##UserSettingsHeader", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    const float pad_x = 12.0f;
    const float y_center = (header_h - ImGui::GetTextLineHeight()) * 0.5f;

    // Back button on left
    ImGui::SetCursorPos(ImVec2(pad_x, y_center));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.05f));

    if (ImGui::Button("< Back") && callbacks.on_back)
    {
        callbacks.on_back();
    }

    ImGui::PopStyleColor(3);

    // Title centered
    const char* title = "Session Settings";
    float title_width = ImGui::CalcTextSize(title).x;
    float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPos(ImVec2((window_width - title_width) * 0.5f, y_center));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();

    // Proceed button on right
    const ImVec2 btn_size(140, 36);
    const float btn_y = (header_h - btn_size.y) * 0.5f;
    const float btn_x = window_width - pad_x - btn_size.x;

    ImGui::SetCursorPos(ImVec2(btn_x, btn_y));

    ImVec4 btn_color = data.can_proceed ? blue : gray;
    ImVec4 btn_hover = data.can_proceed ? blue_h : gray_h;
    ImVec4 txt_color = data.can_proceed ? ImVec4(1, 1, 1, 1) : ImVec4(0.5f, 0.52f, 0.55f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btn_hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, btn_color);
    ImGui::PushStyleColor(ImGuiCol_Text, txt_color);

    if (ImGui::Button("IMPEDANCE", btn_size))
    {
        if (data.can_proceed && callbacks.on_proceed)
        {
            callbacks.on_proceed();
        }
    }

    ImGui::PopStyleColor(4);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    return header_h;
}

}  // namespace elda::views::user_settings