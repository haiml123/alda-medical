#include "monitoring_toolbar.h"
#include "imgui.h"
#include <cmath>

namespace elda {

float MonitoringToolbar(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks) {
    const float header_h = 52.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    // Colors
    const ImVec4 blue    = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blueH   = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 green   = ImVec4(0.20f, 0.90f, 0.35f, 1.00f);
    const ImVec4 greenH  = ImVec4(0.18f, 0.80f, 0.32f, 1.00f);
    const ImVec4 orange  = ImVec4(0.95f, 0.75f, 0.25f, 1.00f);
    const ImVec4 orangeH = ImVec4(0.92f, 0.70f, 0.22f, 1.00f);
    const ImVec4 red     = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    const ImVec4 redH    = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);

    // Status dot helper
    auto DrawStatusDot = [](bool recordingActive, bool paused, float radius) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 rMin = ImGui::GetItemRectMin();
        ImVec2 rMax = ImGui::GetItemRectMax();
        float cx = std::floor((rMin.x + rMax.x) * 0.5f + 0.5f);
        float textOffsetY = ImGui::GetStyle().FramePadding.y;
        float textHeight = ImGui::GetTextLineHeight();
        float cy = rMin.y + textOffsetY + textHeight * 0.5f;

        ImU32 col;
        if (recordingActive) {
            const float PI = 3.14159265358979f;
            float t = (float)ImGui::GetTime();
            float pulse = 0.5f * (1.0f + std::sinf(2.0f * PI * t));
            float alpha = 0.35f + 0.65f * pulse;
            col = ImGui::GetColorU32(ImVec4(0.20f, 0.90f, 0.35f, alpha));
        } else if (paused) {
            col = ImGui::GetColorU32(ImVec4(0.95f, 0.75f, 0.25f, 1.0f));
        } else {
            col = ImGui::GetColorU32(ImVec4(0.65f, 0.65f, 0.65f, 0.85f));
        }
        dl->AddCircleFilled(ImVec2(cx, cy), radius, col, 32);
        dl->AddCircle(ImVec2(cx, cy), radius, ImGui::GetColorU32(ImVec4(0,0,0,0.45f)), 32, 1.0f);
    };

    // Monitor button
    const char* monLabel = data.monitoring ? "STOP MONITOR (F5)" : "MONITOR (F5)";
    ImVec4 monCol = data.monitoring ? red : blue;
    ImVec4 monColH = data.monitoring ? redH : blueH;

    ImGui::PushStyleColor(ImGuiCol_Button, monCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, monColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, monColH);
    if (ImGui::Button(monLabel, ImVec2(160, 36))) {
        if (callbacks.onToggleMonitoring) {
            callbacks.onToggleMonitoring();
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();

    // Record button
    const char* recLabel = data.recordingActive ? "PAUSE (F7)" : "RECORD (F7)";
    ImVec4 recCol = data.recordingActive ? orange : green;
    ImVec4 recColH = data.recordingActive ? orangeH : greenH;

    ImGui::PushStyleColor(ImGuiCol_Button, recCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, recColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, recColH);
    if (data.canRecord) {
        if (ImGui::Button(recLabel, ImVec2(150, 36))) {
            if (callbacks.onToggleRecording) {
                callbacks.onToggleRecording();
            }
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("RECORD (F7)", ImVec2(150, 36));
        ImGui::EndDisabled();
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();

    ImGui::TextDisabled("|");
    ImGui::SameLine();

    // Window controls
    if (ImGui::Button("-##win")) {
        if (callbacks.onDecreaseWindow) {
            callbacks.onDecreaseWindow();
        }
    }
    ImGui::SameLine();
    ImGui::Text("%d sec", data.windowSeconds);
    ImGui::SameLine();
    if (ImGui::Button("+##win")) {
        if (callbacks.onIncreaseWindow) {
            callbacks.onIncreaseWindow();
        }
    }

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12, 1));
    ImGui::SameLine();

    // Amplitude controls
    if (ImGui::Button("-##amp")) {
        if (callbacks.onDecreaseAmplitude) {
            callbacks.onDecreaseAmplitude();
        }
    }
    ImGui::SameLine();
    if (data.amplitudeMicroVolts == 1000) {
        ImGui::Text("1 mV");
    } else {
        ImGui::Text("%d µV", data.amplitudeMicroVolts);
    }
    ImGui::SameLine();
    if (ImGui::Button("+##amp")) {
        if (callbacks.onIncreaseAmplitude) {
            callbacks.onIncreaseAmplitude();
        }
    }

    // Status info (right-aligned)
    ImGui::SameLine(0.0f, 0.0f);
    float right = ImGui::GetWindowContentRegionMax().x;
    float x = right - 360.0f;
    ImGui::SameLine(x > 0 ? x : 0);

    ImGui::Text("%s", data.monitoring ? "MONITORING" : "IDLE");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    const float dotRadius = 5.5f;
    ImGui::InvisibleButton("##rec_dot", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
    DrawStatusDot(data.recordingActive, data.currentlyPaused, dotRadius);
    ImGui::SameLine(0.0f, 6.0f);

    if (data.recordingActive) {
        ImGui::TextColored(green, "RECORDING");
    } else if (data.currentlyPaused) {
        ImGui::TextColored(orange, "PAUSED");
    } else {
        ImGui::TextDisabled("NOT RECORDING");
    }

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    ImGui::Text("%.0f Hz", data.sampleRateHz);
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);

    ImGui::EndChild();

    return header_h;
}

} // namespace elda