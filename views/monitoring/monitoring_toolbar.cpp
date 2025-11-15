#include "monitoring_toolbar.h"
#include "imgui.h"

namespace elda::views::monitoring {

// ======================= small utilities =======================
static inline void center_to_toolbar_y(float toolbar_h, float item_h) {
    ImGui::SetCursorPosY(std::max(0.0f, (toolbar_h - item_h) * 0.5f));
}

// Draw pulsing status dot
static void draw_status_dot(bool recording_active, bool paused, float radius) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();
    float cx = std::floor((rMin.x + rMax.x) * 0.5f + 0.5f);
    float text_offset_y = ImGui::GetStyle().FramePadding.y;
    float text_height = ImGui::GetTextLineHeight();
    float cy = rMin.y + text_offset_y + text_height * 0.5f;

    ImU32 col;
    if (recording_active) {
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
static void render_monitor_button(const MonitoringViewData& data,
                                const MonitoringViewCallbacks& callbacks,
                                float toolbar_h) {
    const ImVec4 blue    = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    const ImVec4 blueH   = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    const ImVec4 red     = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    const ImVec4 redH    = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);

    const char* mon_label = data.monitoring ? "STOP MONITOR (F5)" : "MONITOR (F5)";
    ImVec4 mon_col  = data.monitoring ? red : blue;
    ImVec4 mon_col_h = data.monitoring ? redH : blueH;

    const ImVec2 sz(160, 36);
    center_to_toolbar_y(toolbar_h, sz.y);
    ImGui::PushStyleColor(ImGuiCol_Button, mon_col);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mon_col_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, mon_col_h);
    if (ImGui::Button(mon_label, sz)) {
        if (callbacks.on_toggle_monitoring) callbacks.on_toggle_monitoring();
    }
    ImGui::PopStyleColor(3);
}

// ======================= vector icon helpers =======================
static void draw_play_icon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float w = h * 0.85f;
    ImVec2 p1(center.x - w*0.35f, center.y - h*0.50f);
    ImVec2 p2(center.x - w*0.35f, center.y + h*0.50f);
    ImVec2 p3(center.x + w*0.55f, center.y);
    dl->AddTriangleFilled(p1, p2, p3, col);
}

static void draw_pause_icon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float bar_w = h * 0.22f;
    float gap  = h * 0.18f;
    float r_h   = h * 0.52f;
    ImVec2 a1(center.x - gap - bar_w, center.y - r_h);
    ImVec2 a2(center.x - gap,         center.y + r_h);
    ImVec2 b1(center.x + gap,         center.y - r_h);
    ImVec2 b2(center.x + gap + bar_w,  center.y + r_h);
    dl->AddRectFilled(a1, a2, col, 2.5f);
    dl->AddRectFilled(b1, b2, col, 2.5f);
}

static void draw_stop_icon(ImDrawList* dl, ImVec2 center, float h, ImU32 col) {
    float s = h * 0.70f;
    ImVec2 p1(center.x - s*0.5f, center.y - s*0.5f);
    ImVec2 p2(center.x + s*0.5f, center.y + s*0.5f);
    dl->AddRectFilled(p1, p2, col, 3.0f);
}

// Invisible hit target + subtle hover halo + centered drawing
static bool icon_button(const char* id, ImVec2 size, ImU32 halo_col, float halo_rounding,
                       float icon_scale,
                       const std::function<void(ImDrawList*, ImVec2, ImVec2, float)>& draw_icon)
{
    bool pressed = ImGui::InvisibleButton(id, size);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    if (ImGui::IsItemHovered())
        dl->AddRectFilled(min, max, halo_col, halo_rounding);

    draw_icon(dl, min, max, icon_scale);
    return pressed;
}

// ======================= recording cluster =======================
static void render_recording_controls(const MonitoringViewData& data,
                                    const MonitoringViewCallbacks& callbacks,
                                    float toolbar_h)
{
    const bool is_recording = data.recording_active && data.recording_state == RecordingState::Recording;
    const bool is_paused    = data.recording_active &&  data.recording_state == RecordingState::Paused;
    const bool can_record   = data.can_record;
    const bool can_stop     = is_recording || is_paused;

    // Colors
    const ImU32 col_play   = ImGui::GetColorU32(ImVec4(0.10f, 0.85f, 0.25f, 1.0f)); // dark green
    const ImU32 col_pause  = ImGui::GetColorU32(ImVec4(0.95f, 0.65f, 0.25f, 1.0f)); // dark orange
    const ImU32 col_stop   = ImGui::GetColorU32(ImVec4(0.89f, 0.33f, 0.30f, 1.0f)); // red
    const ImU32 halo_col   = ImGui::GetColorU32(ImVec4(1,1,1,0.06f));
    const float halo_round = 6.0f;

    // Choose a compact size, but center to toolbar height
    const ImVec2 icon_size(20, 18);
    const float  icon_scale = 0.90f; // shrink glyph inside hit area

    ImGui::SameLine();
    center_to_toolbar_y(toolbar_h, icon_size.y);

    // Play/Pause
    if (!can_record) ImGui::BeginDisabled();
    bool toggled = icon_button("##rec_toggle", icon_size, halo_col, halo_round, icon_scale,
        [&](ImDrawList* dl, ImVec2 min, ImVec2 max, float scale){
            ImVec2 center = ImVec2((min.x+max.x)*0.5f, (min.y+max.y)*0.5f);
            float h = (max.y - min.y) * scale;
            if (data.recording_state == RecordingState::Recording) draw_pause_icon(dl, center, h, col_pause);
            else                         draw_play_icon (dl, center, h, col_play);
        });
    if (!can_record) ImGui::EndDisabled();
    if (toggled && callbacks.on_toggle_recording) callbacks.on_toggle_recording();

    // Stop
    ImGui::SameLine();
    center_to_toolbar_y(toolbar_h, icon_size.y);
    if (!can_stop) ImGui::BeginDisabled();
    bool stop_pressed = icon_button("##rec_stop", icon_size, halo_col, halo_round, icon_scale,
        [&](ImDrawList* dl, ImVec2 min, ImVec2 max, float scale){
            ImVec2 center = ImVec2((min.x+max.x)*0.5f, (min.y+max.y)*0.5f);
            float h = (max.y - min.y) * scale;
            draw_stop_icon(dl, center, h, col_stop);
        });
    if (!can_stop) ImGui::EndDisabled();
    if (stop_pressed && callbacks.on_stop_recording) callbacks.on_stop_recording();
}

// Shared helper: square button sized to frame height (keeps all rows consistent)
static bool square_button(const char* id, float pad_extra_x = 0.0f) {
    const float h = ImGui::GetFrameHeight();        // matches other framed widgets
    return ImGui::Button(id, ImVec2(h + pad_extra_x, h));
}

// -----------------------------------------------------------------------------
// Window Duration Controls (fixed)
// -----------------------------------------------------------------------------
static void render_window_controls(const MonitoringViewData& data,
                                 const MonitoringViewCallbacks& callbacks,
                                 float /*toolbar_h*/)
{
    ImGui::SameLine();
    ImGui::TextDisabled("|");

    ImGui::SameLine();
    if (square_button("-##win")) {
        if (callbacks.on_decrease_window) callbacks.on_decrease_window();
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();               // << aligns label with buttons
    ImGui::Text("%d sec", data.window_seconds);

    ImGui::SameLine();
    if (square_button("+##win")) {
        if (callbacks.on_increase_window) callbacks.on_increase_window();
    }
}

// -----------------------------------------------------------------------------
// Amplitude Controls (fixed)
// -----------------------------------------------------------------------------
static void render_amplitude_controls(const MonitoringViewData& data,
                                    const MonitoringViewCallbacks& callbacks,
                                    float /*toolbar_h*/)
{
    ImGui::SameLine(); ImGui::Dummy(ImVec2(12,1)); ImGui::SameLine();

    if (square_button("-##amp")) {
        if (callbacks.on_decrease_amplitude) callbacks.on_decrease_amplitude();
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();               // << aligns label with buttons
    if (data.amplitude_micro_volts == 1000) ImGui::Text("1 mV");
    else                                  ImGui::Text("%d µV", data.amplitude_micro_volts);

    ImGui::SameLine();
    if (square_button("+##amp")) {
        if (callbacks.on_increase_amplitude) callbacks.on_increase_amplitude();
    }
}


    // -----------------------------------------------------------------------------
    // Section: Impedance Viewer Ω Button (perfectly centered icon)
    // -----------------------------------------------------------------------------
    static void render_impedance_button(const MonitoringViewCallbacks& callbacks)
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
        if (callbacks.on_open_impedance_viewer) callbacks.on_open_impedance_viewer();
    }

    // Overlay centered Ω manually (drawn inside button rect)
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);

    // Measure text size and offset so it's visually centered
    const char* label = u8"Ω";
    ImVec2 text_size = ImGui::CalcTextSize(label);
    ImVec2 text_pos(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f + 1.0f); // +1 = visual tweak
    dl->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

    ImGui::PopStyleColor(3);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Open Impedance Viewer");
}


// ======================= status (right) =======================
static void render_status_info(const MonitoringViewData& data, float /*toolbar_h*/) {
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

    const float dot_radius = 5.5f;
    ImGui::InvisibleButton("##rec_dot", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
    draw_status_dot(data.currently_recording, data.currently_paused, dot_radius);
    ImGui::SameLine(0.0f, 6.0f);

    if (data.recording_state == RecordingState::Recording) ImGui::TextColored(green, "RECORDING");
    else if (data.recording_state == RecordingState::Paused) ImGui::TextColored(orange, "PAUSED");
    else                           ImGui::TextDisabled("NOT RECORDING");

    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
    ImGui::Text("%.0f Hz", ImGui::GetIO().Framerate > 0 ? data.sample_rate_hz : data.sample_rate_hz);
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);
}

// ======================= main toolbar =======================
float MonitoringToolbar(const MonitoringViewData& data,
                        const MonitoringViewCallbacks& callbacks) {
    const float header_h = 52.0f;
    ImGui::BeginChild("header", ImVec2(0, header_h), false, ImGuiWindowFlags_NoScrollbar);

    render_monitor_button   (data, callbacks, header_h);
    render_recording_controls(data, callbacks, header_h);
    render_window_controls  (data, callbacks, header_h);
    render_amplitude_controls(data, callbacks, header_h);
    render_impedance_button (callbacks);
    render_status_info      (data, header_h);

    ImGui::EndChild();
    return header_h;
}

} // namespace elda
