#include "eeg_toolbar.h"
#include <cstdio>
#include <cmath>

static inline float Roundf(float x){ return std::floor(x + 0.5f); }

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

// Callback for when user confirms channel selection
static void OnChannelSelectionConfirmed(const std::string& name, const std::vector<Channel>& channels) {
    std::printf("[Channel Selector] Configuration: %s\n", name.c_str());
    std::printf("[Channel Selector] Selected channels:\n");
    for (const auto& ch : channels) {
        if (ch.selected) {
            std::printf("  - %s (ID: %s, Color: %s)\n",
                       ch.name.c_str(), ch.id.c_str(), ch.color.c_str());
        }
    }
}

float DrawToolbar(AppState& st) {
    // Static modal instance
    static ChannelSelectorModal channelModal;

    const float header_h = 52.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    // Palette
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
    const bool monitoring = st.isMonitoring;
    const char* monLabel  = monitoring ? "STOP MONITOR (F5)" : "MONITOR (F5)";
    ImVec4 monCol         = monitoring ? red : blue;
    ImVec4 monColH        = monitoring ? redH : blueH;

    ImGui::PushStyleColor(ImGuiCol_Button,        monCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, monColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  monColH);
    if (ImGui::Button(monLabel, ImVec2(160, 36))) {
        if (monitoring) {
            st.isMonitoring = false;
            if (st.isRecordingToFile) {
                st.isRecordingToFile = false;
                st.isPaused = false;
                std::printf("[MONITOR OFF] Also stopped recording at t=%.3f s\n", st.currentEEGTime());
            } else {
                std::printf("[MONITOR OFF] Monitoring stopped at t=%.3f s\n", st.currentEEGTime());
            }
        } else {
            st.isMonitoring = true;
            std::printf("[MONITOR ON] Monitoring started at t=%.3f s\n", st.currentEEGTime());
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // ===== RECORD / PAUSE toggle =====
    const bool canRecord       = st.isMonitoring;
    const bool recordingActive = st.isRecordingToFile && !st.isPaused;
    const bool currentlyPaused = st.isRecordingToFile &&  st.isPaused;

    const char* recLabel = recordingActive ? "PAUSE (F7)" : "RECORD (F7)";
    ImVec4 recCol        = recordingActive ? orange : green;
    ImVec4 recColH       = recordingActive ? orangeH : greenH;

    ImGui::PushStyleColor(ImGuiCol_Button,        recCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, recColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  recColH);
    if (canRecord) {
        if (ImGui::Button(recLabel, ImVec2(150, 36))) {
            if (recordingActive) {
                st.isPaused = true;
                st.pauseMarks.push_back({ st.currentEEGTime() });
                std::printf("[PAUSE] Recording paused at t=%.3f s\n", st.currentEEGTime());
            } else {
                st.isRecordingToFile = true;
                st.isPaused          = false;
                std::printf("%s recording at t=%.3f s\n",
                            currentlyPaused ? "[RESUME]" : "[RECORD]", st.currentEEGTime());
            }
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button(recLabel, ImVec2(150, 36));
        ImGui::EndDisabled();
    }
    ImGui::PopStyleColor(3);

    // Divider
    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();

    // Window - / value / +
    if (ImGui::Button("-")) { decIdx(st.winIdx, WINDOW_COUNT); }
    ImGui::SameLine();
    ImGui::Text("%d sec", (int)WINDOW_OPTIONS[st.winIdx]);
    ImGui::SameLine();
    if (ImGui::Button("+")) { incIdx(st.winIdx, WINDOW_COUNT); }

    ImGui::SameLine(); ImGui::Dummy(ImVec2(12,1)); ImGui::SameLine();

    // Amplitude (pp) - / value / +
    if (ImGui::Button("-##amp")) { decIdx(st.ampIdx, AMP_COUNT); }
    ImGui::SameLine();
    {
        int amp = AMP_PP_UV_OPTIONS[st.ampIdx];
        if (amp == 1000) ImGui::Text("1 mV");
        else             ImGui::Text("%d \xC2\xB5V", amp);
    }
    ImGui::SameLine();
    if (ImGui::Button("+##amp")) { incIdx(st.ampIdx, AMP_COUNT); }

    // Channel Selector button
    ImGui::SameLine(); ImGui::Dummy(ImVec2(12,1)); ImGui::SameLine();

    // Store button position BEFORE drawing the button
    ImVec2 channelButtonPos = ImGui::GetCursorScreenPos();
    ImVec2 channelButtonSize = ImVec2(120, 36);

    ImGui::PushStyleColor(ImGuiCol_Button,        purple);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, purpleH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  purpleH);
    if (ImGui::Button("Channels", channelButtonSize)) {
        channelModal.Open(&st.availableChannels, OnChannelSelectionConfirmed);
    }
    ImGui::PopStyleColor(3);

    // ===== Right status area =====
    ImGui::SameLine(0.0f, 0.0f);
    float right = ImGui::GetWindowContentRegionMax().x;
    float x = right - 360.0f;
    ImGui::SameLine(x > 0 ? x : 0);

    // Monitoring text
    ImGui::Text("%s", st.isMonitoring ? "MONITORING" : "IDLE");
    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();

    // Status dot
    const float dotRadius = 5.5f;
    ImGui::InvisibleButton("##rec_dot", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
    DrawStatusDotInLastItem(recordingActive, currentlyPaused, dotRadius);
    ImGui::SameLine(0.0f, 6.0f);

    // Recording status text
    if (st.isRecordingToFile) {
        if (st.isPaused) ImGui::TextColored(orange, "PAUSED");
        else             ImGui::TextColored(green,  "RECORDING");
    } else {
        ImGui::TextDisabled("NOT RECORDING");
    }

    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
    ImGui::Text("%.0f Hz", SAMPLE_RATE_HZ);
    ImGui::SameLine(); ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);

    ImGui::EndChild();

    // Draw the channel modal
    channelModal.Draw(channelButtonPos, channelButtonSize);

    return header_h;
}