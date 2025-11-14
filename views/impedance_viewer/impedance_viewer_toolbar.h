#pragma once
#include "imgui.h"
#include "impedance_viewer_view.h"
#include <algorithm>
#include <cmath>

namespace elda::views::impedance_viewer {

    static inline void CenterToToolbarY(float toolbar_h, float item_h) {
        ImGui::SetCursorPosY(std::max(0.0f, (toolbar_h - item_h) * 0.5f));
    }

    static void RenderRightButtons(const ImpedanceViewerViewCallbacks& callbacks, float toolbar_h) {
        const ImVec4 gray   = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
        const ImVec4 grayH  = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);
        const ImVec4 blue   = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
        const ImVec4 blueH  = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);

        const ImVec2 btnSize(100, 36);
        const float gap = 8.0f;
        const float padX = 12.0f;

        // Calculate position for right-aligned buttons
        const float groupWidth = btnSize.x * 2.0f + gap;
        const float availWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        const float xRight = availWidth - padX - groupWidth;
        const float yCenter = (toolbar_h - btnSize.y) * 0.5f;

        // Settings button
        ImGui::SetCursorPos(ImVec2(xRight, yCenter));
        ImGui::PushStyleColor(ImGuiCol_Button, gray);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, grayH);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, grayH);
        if (ImGui::Button("SETTINGS", btnSize)) {
            if (callbacks.onRedirectToSettings) callbacks.onRedirectToSettings();
        }
        ImGui::PopStyleColor(3);

        // OK button
        ImGui::SetCursorPos(ImVec2(xRight + btnSize.x + gap, yCenter));
        ImGui::PushStyleColor(ImGuiCol_Button, blue);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blueH);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, blueH);
        if (ImGui::Button("OK", btnSize)) {
            // if (callbacks.onSave) callbacks.onSave();
            if (callbacks.onRedirectToMonitoring) callbacks.onRedirectToMonitoring();
        }
        ImGui::PopStyleColor(3);
    }

    inline float ImpedanceViewerToolbar(const ImpedanceViewerViewCallbacks& callbacks) {
        const float header_h = 52.0f;
        const ImVec4 plotBg = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, plotBg);
        ImGui::BeginChild("impedance_header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

        RenderRightButtons(callbacks, header_h);

        ImGui::EndChild();
        ImGui::PopStyleColor();
        return header_h;
    }

} // namespace elda::impedance_viewer