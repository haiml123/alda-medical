#pragma once

#include "imgui.h"

#include <functional>
#include <string>
#include <vector>

namespace elda::ui
{

struct HeaderButton
{
    std::string label;
    std::function<void()> on_click;
    bool enabled = true;
    bool primary = false;  // true = blue, false = gray
    float width = 100.0f;
};

struct ScreenHeaderConfig
{
    std::string title;
    bool show_back_button = true;
    std::function<void()> on_back;
    std::vector<HeaderButton> buttons;
    float height = 52.0f;
};

inline float render_screen_header(const ScreenHeaderConfig& config)
{
    const float header_h = config.height;
    const ImVec4 header_bg = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

    const ImVec4 blue = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blue_h = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 gray = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    const ImVec4 gray_h = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, header_bg);
    ImGui::BeginChild("##ScreenHeader", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    const float pad_x = 12.0f;
    const float y_center = (header_h - ImGui::GetTextLineHeight()) * 0.5f;
    const float btn_height = 36.0f;
    const float btn_y = (header_h - btn_height) * 0.5f;

    // Back button on left
    if (config.show_back_button)
    {
        ImGui::SetCursorPos(ImVec2(pad_x, y_center));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.05f));

        if (ImGui::Button("< Back") && config.on_back)
        {
            config.on_back();
        }

        ImGui::PopStyleColor(3);
    }

    // Title centered
    if (!config.title.empty())
    {
        float title_width = ImGui::CalcTextSize(config.title.c_str()).x;
        float window_width = ImGui::GetWindowSize().x;
        ImGui::SetCursorPos(ImVec2((window_width - title_width) * 0.5f, y_center));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
        ImGui::Text("%s", config.title.c_str());
        ImGui::PopStyleColor();
    }

    // Buttons on right
    if (!config.buttons.empty())
    {
        float window_width = ImGui::GetWindowSize().x;
        const float gap = 8.0f;

        // Calculate total buttons width
        float total_width = 0;
        for (const auto& btn : config.buttons)
        {
            total_width += btn.width;
        }
        total_width += gap * (config.buttons.size() - 1);

        float btn_x = window_width - pad_x - total_width;

        for (const auto& btn : config.buttons)
        {
            ImGui::SetCursorPos(ImVec2(btn_x, btn_y));

            ImVec4 btn_color = btn.primary ? blue : gray;
            ImVec4 btn_hover = btn.primary ? blue_h : gray_h;

            if (!btn.enabled)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            }

            ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btn.enabled ? btn_hover : btn_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, btn_color);

            if (ImGui::Button(btn.label.c_str(), ImVec2(btn.width, btn_height)))
            {
                if (btn.enabled && btn.on_click)
                {
                    btn.on_click();
                }
            }

            ImGui::PopStyleColor(3);

            if (!btn.enabled)
            {
                ImGui::PopStyleVar();
            }

            btn_x += btn.width + gap;
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    return header_h;
}

}  // namespace elda::ui