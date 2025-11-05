#include "eeg_toolbar.h"

float DrawToolbar(AppState& st) {
    const float header_h = 48.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    // Brand colors (explicit literals)
    const ImVec4 blue  = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blueH = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 red   = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    const ImVec4 redH  = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);

    // Pause/Resume
    ImGui::PushStyleColor(ImGuiCol_Button,        blue);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blueH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  blueH);
    if (st.recording) { if (ImGui::Button("Pause"))  st.recording = false; }
    else              { if (ImGui::Button("Resume")) { st.recording = true; st.stopped=false; } }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Stop
    ImGui::PushStyleColor(ImGuiCol_Button,        red);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, redH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  redH);
    if (ImGui::Button("Stop")) { st.recording=false; st.stopped=true; st.ring.reset(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();

    // Window - / value / +
    if (ImGui::Button("-")) { decIdx(st.winIdx, WINDOW_COUNT); }
    ImGui::SameLine();
    ImGui::Text("%d sec", (int)WINDOW_OPTIONS[st.winIdx]);
    ImGui::SameLine();
    if (ImGui::Button("+")) { incIdx(st.winIdx, WINDOW_COUNT); }

    ImGui::SameLine(); ImGui::Dummy(ImVec2(14,1)); ImGui::SameLine();

    // Amplitude (pp) - / value / +
    if (ImGui::Button("-##amp")) { decIdx(st.ampIdx, AMP_COUNT); }
    ImGui::SameLine();
    int amp = AMP_PP_UV_OPTIONS[st.ampIdx];
    if (amp == 1000) ImGui::Text("1 mV");
    else             ImGui::Text("%d \xC2\xB5V", amp);
    ImGui::SameLine();
    if (ImGui::Button("+##amp")) { incIdx(st.ampIdx, AMP_COUNT); }

    ImGui::SameLine(); ImGui::Dummy(ImVec2(14,1)); ImGui::SameLine();
    ImGui::Text("%.0f Hz", SAMPLE_RATE_HZ);

    // Right status
    ImGui::SameLine(0.0f, 0.0f);
    float right = ImGui::GetWindowContentRegionMax().x;
    float x = right - 360.0f;
    ImGui::SameLine(x > 0 ? x : 0);
    ImGui::TextColored(ImVec4(0.8f,0.3f,0.3f,1),
        st.recording ? "\xE2\x97\x8F RECORDING" : (st.stopped ? "\xE2\x97\x8F STOPPED" : "\xE2\x97\x8F PAUSED"));
    ImGui::SameLine();
    ImGui::Text("%d/%d channels", CHANNELS, 64);
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);

    ImGui::EndChild();
    return header_h;
}
