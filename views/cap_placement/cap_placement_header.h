#pragma once

#include "imgui.h"
#include <functional>

namespace elda::views::cap_placement {

struct CapPlacementHeaderCallbacks {
    std::function<void()> on_proceed;
    std::function<void()> on_back;
};

struct CapPlacementHeaderData {
    bool can_proceed = true;
    size_t current_step = 0;
    size_t total_steps = 1;
};

inline float render_cap_placement_header(
    const CapPlacementHeaderData& data,
    const CapPlacementHeaderCallbacks& callbacks
) {
    const float header_h = 52.0f;
    const ImVec4 header_bg = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);
    
    const ImVec4 blue   = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blue_h = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, header_bg);
    ImGui::BeginChild("##CapPlacementHeader", ImVec2(0, header_h), false,
                      ImGuiWindowFlags_NoScrollbar);

    const float pad_x = 12.0f;
    const float y_center = (header_h - ImGui::GetTextLineHeight()) * 0.5f;

    // Back button on left
    ImGui::SetCursorPos(ImVec2(pad_x, y_center));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.05f));
    
    if (ImGui::Button("< Back") && callbacks.on_back) {
        callbacks.on_back();
    }
    
    ImGui::PopStyleColor(3);

    // Title centered
    const char* title = "Cap Placement Guide";
    float title_width = ImGui::CalcTextSize(title).x;
    float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPos(ImVec2((window_width - title_width) * 0.5f, y_center));
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();

    // Step indicator and OK button on right
    const ImVec2 btn_size(100, 36);
    const float btn_y = (header_h - btn_size.y) * 0.5f;
    
    // Step indicator (e.g., "3 / 6")
    char step_text[32];
    snprintf(step_text, sizeof(step_text), "%zu / %zu", 
             data.current_step + 1, data.total_steps);
    float step_width = ImGui::CalcTextSize(step_text).x;
    
    const float btn_x = window_width - pad_x - btn_size.x;
    const float step_x = btn_x - step_width - 20.0f;
    
    ImGui::SetCursorPos(ImVec2(step_x, y_center));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.57f, 0.60f, 1.0f));
    ImGui::Text("%s", step_text);
    ImGui::PopStyleColor();

    // OK button
    ImGui::SetCursorPos(ImVec2(btn_x, btn_y));

    ImGui::PushStyleColor(ImGuiCol_Button, blue);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blue_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, blue);

    if (ImGui::Button("OK", btn_size)) {
        if (callbacks.on_proceed) {
            callbacks.on_proceed();
        }
    }

    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    return header_h;
}

} // namespace elda::views::cap_placement
