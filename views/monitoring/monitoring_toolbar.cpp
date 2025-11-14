#include "monitoring_toolbar.h"
#include "imgui.h"

namespace elda::views::monitoring {

// ======================= small utilities =======================
static inline void CenterToToolbarY(float toolbar_h, float item_h) {
    ImGui::SetCursorPosY(std::max(0.0f, (toolbar_h - item_h) * 0.5f));
}

// Draw pulsing status dot
static void DrawStatusDot(bool recordingActive, bool paused, float radius) {
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
}

// ======================= monitor button =======================
static void RenderMonitorButton(const MonitoringViewData& data,
                                const MonitoringViewCallbacks& callbacks,
                                float toolbar_h) {
    const ImVec4 blue    = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blueH   = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 red     = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    const ImVec4 redH    = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);

    const char* monLabel = data.monitoring ? "STOP MONITOR (F5)" : "MONITOR (F5)";
    ImVec4 monCol  = data.monitoring ? red : blue;
    ImVec4 monColH = data.monitoring ? redH : blueH;

    const ImVec2 sz(160, 36);
    CenterToToolbarY(toolbar_h, sz.y);
    ImGui::PushStyleColor(ImGuiCol_Button, monCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, monColH);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, monColH);
    if (ImGui::Button(monLabel, sz)) {
        if (callbacks.onToggleMonitoring) callbacks.onToggleMonitoring();
    }
    ImGui::PopStyleColor(3);
}

// ======================= vector icon helpers =======================
static void DrawPlayIcon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float w = h * 0.85f;
    ImVec2 p1(center.x - w*0.35f, center.y - h*0.50f);
    ImVec2 p2(center.x - w*0.35f, center.y + h*0.50f);
    ImVec2 p3(center.x + w*0.55f, center.y);
    dl->AddTriangleFilled(p1, p2, p3, col);
}

static void DrawPauseIcon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float barW = h * 0.22f;
    float gap  = h * 0.18f;
    float rH   = h * 0.52f;
    ImVec2 a1(center.x - gap - barW, center.y - rH);
    ImVec2 a2(center.x - gap,         center.y + rH);
    ImVec2 b1(center.x + gap,         center.y - rH);
    ImVec2 b2(center.x + gap + barW,  center.y + rH);
    dl->AddRectFilled(a1, a2, col, 2.5f);
    dl->AddRectFilled(b1, b2, col, 2.5f);
}

static void DrawStopIcon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float s = h * 0.70f;
    ImVec2 p1(center.x - s*0.5f, center.y - s*0.5f);
    ImVec2 p2(center.x + s*0.5f, center.y + s*0.5f);
    dl->AddRectFilled(p1, p2, col, 3.0f);
}

// Invisible hit target + subtle hover halo + centered drawing
static bool IconButton(const char* id, ImVec2 size, ImU32 haloCol, float haloRounding,
                       float iconScale,
                       const std::function<void(ImDrawList*, ImVec2, ImVec2, float)>& drawIcon)
{
    bool pressed = ImGui::InvisibleButton(id, size);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    if (ImGui::IsItemHovered())
        dl->AddRectFilled(min, max, haloCol, haloRounding);

    drawIcon(dl, min, max, iconScale);
    return pressed;
}

// ======================= recording cluster =======================
static void RenderRecordingControls(const MonitoringViewData& data,
                                    const MonitoringViewCallbacks& callbacks,
                                    float toolbar_h)
{
    const bool isRecording = data.recordingActive && data.recordingState == RecordingState::Recording;
    const bool isPaused    = data.recordingActive &&  data.recordingState == RecordingState::Paused;
    const bool canRecord   = data.canRecord;
    const bool canStop     = isRecording || isPaused;

    // Colors
    const ImU32 colPlay   = ImGui::GetColorU32(ImVec4(0.10f, 0.85f, 0.25f, 1.0f)); // dark green
    const ImU32 colPause  = ImGui::GetColorU32(ImVec4(0.95f, 0.65f, 0.25f, 1.0f)); // dark orange
    const ImU32 colStop   = ImGui::GetColorU32(ImVec4(0.89f, 0.33f, 0.30f, 1.0f)); // red
    const ImU32 haloCol   = ImGui::GetColorU32(ImVec4(1,1,1,0.06f));
    const float haloRound = 6.0f;

    // Choose a compact size, but center to toolbar height
    const ImVec2 iconSize(20, 18);
    const float  iconScale = 0.90f; // shrink glyph inside hit area

    ImGui::SameLine();
    CenterToToolbarY(toolbar_h, iconSize.y);

    // Play/Pause
    if (!canRecord) ImGui::BeginDisabled();
    bool toggled = IconButton("##rec_toggle", iconSize, haloCol, haloRound, iconScale,
        [&](ImDrawList* dl, ImVec2 min, ImVec2 max, float scale){
            ImVec2 center = ImVec2((min.x+max.x)*0.5f, (min.y+max.y)*0.5f);
            float h = (max.y - min.y) * scale;
            if (data.recordingState == RecordingState::Recording) DrawPauseIcon(dl, center, h, colPause);
            else                         DrawPlayIcon (dl, center, h, colPlay);
        });
    if (!canRecord) ImGui::EndDisabled();
    if (toggled && callbacks.onToggleRecording) callbacks.onToggleRecording();

    // Stop
    ImGui::SameLine();
    CenterToToolbarY(toolbar_h, iconSize.y);
    if (!canStop) ImGui::BeginDisabled();
    bool stopPressed = IconButton("##rec_stop", iconSize, haloCol, haloRound, iconScale,
        [&](ImDrawList* dl, ImVec2 min, ImVec2 max, float scale){
            ImVec2 center = ImVec2((min.x+max.x)*0.5f, (min.y+max.y)*0.5f);
            float h = (max.y - min.y) * scale;
            DrawStopIcon(dl, center, h, colStop);
        });
    if (!canStop) ImGui::EndDisabled();
    if (stopPressed && callbacks.onStopRecording) callbacks.onStopRecording();
}

// Shared helper: square button sized to frame height (keeps all rows consistent)
static bool SquareButton(const char* id, float pad_extra_x = 0.0f) {
    const float h = ImGui::GetFrameHeight();        // matches other framed widgets
    return ImGui::Button(id, ImVec2(h + pad_extra_x, h));
}

// -----------------------------------------------------------------------------
// Window Duration Controls (fixed)
// -----------------------------------------------------------------------------
static void RenderWindowControls(const MonitoringViewData& data,
                                 const MonitoringViewCallbacks& callbacks,
                                 float /*toolbar_h*/)
{
    ImGui::SameLine();
    ImGui::TextDisabled("|");

    ImGui::SameLine();
    if (SquareButton("-##win")) {
        if (callbacks.onDecreaseWindow) callbacks.onDecreaseWindow();
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();               // << aligns label with buttons
    ImGui::Text("%d sec", data.windowSeconds);

    ImGui::SameLine();
    if (SquareButton("+##win")) {
        if (callbacks.onIncreaseWindow) callbacks.onIncreaseWindow();
    }
}

// -----------------------------------------------------------------------------
// Amplitude Controls (fixed)
// -----------------------------------------------------------------------------
static void RenderAmplitudeControls(const MonitoringViewData& data,
                                    const MonitoringViewCallbacks& callbacks,
                                    float /*toolbar_h*/)
{
    ImGui::SameLine(); ImGui::Dummy(ImVec2(12,1)); ImGui::SameLine();

    if (SquareButton("-##amp")) {
        if (callbacks.onDecreaseAmplitude) callbacks.onDecreaseAmplitude();
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();               // << aligns label with buttons
    if (data.amplitudeMicroVolts == 1000) ImGui::Text("1 mV");
    else                                  ImGui::Text("%d µV", data.amplitudeMicroVolts);

    ImGui::SameLine();
    if (SquareButton("+##amp")) {
        if (callbacks.onIncreaseAmplitude) callbacks.onIncreaseAmplitude();
    }
}


    // -----------------------------------------------------------------------------
    // Section: Impedance Viewer Ω Button (perfectly centered icon)
    // -----------------------------------------------------------------------------
    static void RenderImpedanceButton(const MonitoringViewCallbacks& callbacks)
{
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12, 1));
    ImGui::SameLine();

    const float h = ImGui::GetFrameHeight();
    const ImVec2 size(h, h);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImGui::GetStyleColorVec4(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

    // Begin a child region to adjust vertical centering manually
    ImVec2 startPos = ImGui::GetCursorPos();
    if (ImGui::Button("##imp_btn", size)) {
        if (callbacks.onOpenImpedanceViewer) callbacks.onOpenImpedanceViewer();
    }

    // Overlay centered Ω manually (drawn inside button rect)
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);

    // Measure text size and offset so it's visually centered
    const char* label = u8"Ω";
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f + 1.0f); // +1 = visual tweak
    dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label);

    ImGui::PopStyleColor(3);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Open Impedance Viewer");
}


// ======================= status (right) =======================
static void RenderStatusInfo(const MonitoringViewData& data, float /*toolbar_h*/) {
    ImGui::SameLine(0.0f, 0.0f);
    float right = ImGui::GetWindowContentRegionMax().x;
    float x = right - 360.0f;
    ImGui::SameLine(x > 0 ? x : 0);

    const ImVec4 green  = ImVec4(0.20f, 0.90f, 0.35f, 1.00f);
    const ImVec4 orange = ImVec4(0.95f, 0.75f, 0.25f, 1.00f);

    ImGui::Text("%s", data.monitoring ? "MONITORING" : "IDLE");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    const float dotRadius = 5.5f;
    ImGui::InvisibleButton("##rec_dot", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
    DrawStatusDot(data.currentlyRecording, data.currentlyPaused, dotRadius);
    ImGui::SameLine(0.0f, 6.0f);

    if (data.recordingState == RecordingState::Recording) ImGui::TextColored(green, "RECORDING");
    else if (data.recordingState == RecordingState::Paused) ImGui::TextColored(orange, "PAUSED");
    else                           ImGui::TextDisabled("NOT RECORDING");

    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
    ImGui::Text("%.0f Hz", ImGui::GetIO().Framerate > 0 ? data.sampleRateHz : data.sampleRateHz);
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);
}

// ======================= main toolbar =======================
float MonitoringToolbar(const MonitoringViewData& data,
                        const MonitoringViewCallbacks& callbacks) {
    const float header_h = 52.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    RenderMonitorButton   (data, callbacks, header_h);
    RenderRecordingControls(data, callbacks, header_h);
    RenderWindowControls  (data, callbacks, header_h);
    RenderAmplitudeControls(data, callbacks, header_h);
    RenderImpedanceButton (callbacks);
    RenderStatusInfo      (data, header_h);

    ImGui::EndChild();
    return header_h;
}

} // namespace elda
