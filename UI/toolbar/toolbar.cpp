#include "eeg_toolbar.h"
#include <cstdio>
#include <cmath>

// Helper functions
static inline float Roundf(float x) { return std::floor(x + 0.5f); }

static void DrawStatusDotInLastItem(bool recordingActive, bool paused, float radius) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();

    float cx = Roundf((rMin.x + rMax.x) * 0.5f);

    // Align dot to text baseline instead of geometric center
    float textOffsetY = ImGui::GetStyle().FramePadding.y;
    float textHeight = ImGui::GetTextLineHeight();
    float cy = rMin.y + textOffsetY + textHeight * 0.5f;

    // Colors: recording = flashing green, paused = solid orange, idle = dim gray
    ImU32 col;
    if (recordingActive) {
        const float PI = 3.14159265358979f;
        float t = (float)ImGui::GetTime();
        float pulse = 0.5f * (1.0f + std::sinf(2.0f * PI * t)); // 0..1
        float alpha = 0.35f + 0.65f * pulse;                    // 0.35..1.0
        col = ImGui::GetColorU32(ImVec4(0.20f, 0.90f, 0.35f, alpha)); // green
    } else if (paused) {
        col = ImGui::GetColorU32(ImVec4(0.95f, 0.75f, 0.25f, 1.0f));  // orange
    } else {
        col = ImGui::GetColorU32(ImVec4(0.65f, 0.65f, 0.65f, 0.85f)); // gray
    }

    dl->AddCircleFilled(ImVec2(cx, cy), radius, col, 32);
    dl->AddCircle(ImVec2(cx, cy), radius, ImGui::GetColorU32(ImVec4(0,0,0,0.45f)), 32, 1.0f);
}

float Toolbar(AppState& st, elda::AppStateManager& stateManager) {
    // Static presenter instance for MVP pattern
    using namespace elda::channels_group;
    static ChannelsGroupPresenter g_channelsPresenter;

    const float header_h = 52.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    // Color palette
    const ImVec4 blue    = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blueH   = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 green   = ImVec4(0.20f, 0.90f, 0.35f, 1.00f);
    const ImVec4 greenH  = ImVec4(0.18f, 0.80f, 0.32f, 1.00f);
    const ImVec4 orange  = ImVec4(0.95f, 0.75f, 0.25f, 1.00f);
    const ImVec4 orangeH = ImVec4(0.92f, 0.70f, 0.22f, 1.00f);
    const ImVec4 red     = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    const ImVec4 redH    = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);
    const ImVec4 purple  = ImVec4(0.60f, 0.40f, 0.80f, 1.00f);
    const ImVec4 purpleH = ImVec4(0.65f, 0.45f, 0.85f, 1.00f);

    // ===== MONITOR toggle =====
    const bool monitoring = stateManager.IsMonitoring();
    const char* monLabel  = monitoring ? "STOP MONITOR (F5)" : "MONITOR (F5)";
    ImVec4 monCol         = monitoring ? red : blue;
    ImVec4 monColH        = monitoring ? redH : blueH;

    ImGui::PushStyleColor(ImGuiCol_Button,        monCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, monColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  monColH);

    if (ImGui::Button(monLabel, ImVec2(160, 36))) {
        // Use state manager for validation, but handle state changes simply
        auto result = stateManager.SetMonitoring(!monitoring);
        if (!result.IsSuccess()) {
            std::fprintf(stderr, "[UI Error] %s\n", result.message.c_str());
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::SameLine();

    // ===== RECORD / PAUSE toggle =====
    const bool canRecord       = monitoring;
    const bool recordingActive = stateManager.IsRecording() && !stateManager.IsPaused();
    const bool currentlyPaused = stateManager.IsRecording() && stateManager.IsPaused();

    const char* recLabel = recordingActive ? "PAUSE (F7)" : "RECORD (F7)";
    ImVec4 recCol        = recordingActive ? orange : green;
    ImVec4 recColH       = recordingActive ? orangeH : greenH;

    ImGui::PushStyleColor(ImGuiCol_Button,        recCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, recColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  recColH);

    if (canRecord) {
        if (ImGui::Button(recLabel, ImVec2(150, 36))) {
            if (recordingActive) {
                // Pause recording
                auto result = stateManager.PauseRecording();
                if (!result.IsSuccess()) {
                    std::fprintf(stderr, "[RECORD] Failed to pause: %s\n", result.message.c_str());
                }
            } else {
                // Start or resume recording
                elda::StateChangeError result;
                if (currentlyPaused) {
                    result = stateManager.ResumeRecording();
                    if (!result.IsSuccess()) {
                        std::fprintf(stderr, "[RECORD] Failed to resume: %s\n", result.message.c_str());
                    }
                } else {
                    result = stateManager.StartRecording();
                    if (!result.IsSuccess()) {
                        std::fprintf(stderr, "[RECORD] Failed to start: %s\n", result.message.c_str());

                        // Special message for impedance check
                        if (result.result == elda::StateChangeResult::ImpedanceCheckRequired) {
                            std::fprintf(stderr, "[RECORD] HINT: Call stateManager.SetImpedanceCheckPassed(true) in development mode\n");
                        }
                    }
                }
            }
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("RECORD (F7)", ImVec2(150, 36));
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor(3);
    ImGui::SameLine();

    // Divider
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    // ===== Window size controls =====
    if (ImGui::Button("-##win")) {
        if (st.winIdx > 0) {
            stateManager.SetDisplayWindow(st.winIdx - 1);
        }
    }
    ImGui::SameLine();
    ImGui::Text("%d sec", (int)stateManager.GetWindowSeconds());
    ImGui::SameLine();
    if (ImGui::Button("+##win")) {
        if (st.winIdx < WINDOW_COUNT - 1) {
            stateManager.SetDisplayWindow(st.winIdx + 1);
        }
    }

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12, 1));
    ImGui::SameLine();

    // ===== Amplitude controls =====
    if (ImGui::Button("-##amp")) {
        if (st.ampIdx > 0) {
            stateManager.SetDisplayAmplitude(st.ampIdx - 1);
        }
    }
    ImGui::SameLine();
    {
        int amp = stateManager.GetAmplitudeMicroVolts();
        if (amp == 1000) {
            ImGui::Text("1 mV");
        } else {
            ImGui::Text("%d µV", amp);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("+##amp")) {
        if (st.ampIdx < AMP_COUNT - 1) {
            stateManager.SetDisplayAmplitude(st.ampIdx + 1);
        }
    }

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12, 1));
    ImGui::SameLine();

    // ===== CHANNELS button =====
    ImVec2 channelButtonPos = ImGui::GetCursorScreenPos();
    ImVec2 channelButtonSize = ImVec2(120, 36);

    ImGui::PushStyleColor(ImGuiCol_Button, purple);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, purpleH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, purpleH);

    ImGui::PopStyleColor(3);

    // ===== Right status area =====
    ImGui::SameLine(0.0f, 0.0f);
    float right = ImGui::GetWindowContentRegionMax().x;
    float x = right - 360.0f;
    ImGui::SameLine(x > 0 ? x : 0);

    // Monitoring text
    ImGui::Text("%s", monitoring ? "MONITORING" : "IDLE");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    // Status dot
    const float dotRadius = 5.5f;
    ImGui::InvisibleButton("##rec_dot", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
    DrawStatusDotInLastItem(recordingActive, currentlyPaused, dotRadius);
    ImGui::SameLine(0.0f, 6.0f);

    // Recording status text
    if (stateManager.IsRecording()) {
        if (currentlyPaused) {
            ImGui::TextColored(orange, "PAUSED");
        } else {
            ImGui::TextColored(green, "RECORDING");
        }
    } else {
        ImGui::TextDisabled("NOT RECORDING");
    }

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    // Frame rate and sample rate
    ImGui::Text("%.0f Hz", SAMPLE_RATE_HZ);
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);

    ImGui::EndChild();

    // CRITICAL: Render the presenter every frame (even when modal is closed)
    // g_channelsPresenter.Render(channelButtonPos);

    return header_h;
}