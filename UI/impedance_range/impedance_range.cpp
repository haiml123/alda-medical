#include "impedance_range.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace elda::ui
{

static inline float clampf(float v, float a, float b)
{
    return v < a ? a : (v > b ? b : v);
}

static void default_format_ohms(char* buf, int n, float ohms)
{
    // 10,000 -> "10K", 54,000 -> "54K", 2,350 -> "2.35K", <1k -> raw Ohms
    if (ohms >= 1000.0f)
    {
        float k = ohms / 1000.0f;
        if (k >= 10.0f)
            std::snprintf(buf, n, "%.0fK", k);
        else
            std::snprintf(buf, n, "%.2fK", k);
    }
    else
    {
        std::snprintf(buf, n, "%.0f", ohms);
    }
}

static ImGuiID storage_key(const char* id_label, const char* suffix)
{
    ImGui::PushID(id_label);
    ImGuiID key = ImGui::GetID(suffix);
    ImGui::PopID();
    return key;
}

bool draw_impedance_range_dual(const char* id_label,
                               float low_ohms,
                               float high_ohms,
                               const ImpedanceRanges& ranges,
                               const ImpedanceRangeConfig& cfg,
                               const DualCursorConfig& dcfg,
                               float* out_low,
                               float* out_high)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // Layout height (space for cursor labels above)
    float label_h = ImGui::GetTextLineHeight();  // we always display cursor labels now
    float bar_h = cfg.bar_height;
    float total_h = label_h + cfg.top_labels_gap + dcfg.cursor_overhang + bar_h + dcfg.cursor_overhang + 8.0f;
    ImGui::InvisibleButton(id_label, ImVec2(avail.x, total_h));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    // Geometry
    const float x0 = cursor.x + cfg.side_pad;
    const float x1 = cursor.x + avail.x - cfg.side_pad;
    const float y_bar = cursor.y + label_h + cfg.top_labels_gap + dcfg.cursor_overhang;
    const ImVec2 bar_p0(x0, y_bar);
    const ImVec2 bar_p1(x1, y_bar + bar_h);
    const float bar_w = std::max(1.0f, x1 - x0);

    // Clamp scale
    float max_range = std::max(1.0f, ranges.max_range);

    // Map functions
    auto ohms_to_x = [&](float ohms)
    {
        float t = clampf(ohms / max_range, 0.0f, 1.0f);
        return x0 + t * bar_w;
    };
    auto x_to_ohms = [&](float x)
    {
        float t = clampf((x - x0) / bar_w, 0.0f, 1.0f);
        return t * max_range;
    };

    // Current cursor positions
    float x_low = ohms_to_x(low_ohms);
    float x_high = ohms_to_x(high_ohms);

    // --- Dynamic segments based on cursors ---
    // Green: [x0 .. x_low]
    // Yellow:[x_low .. x_high]
    // Red:   [x_high .. x1]
    if (x_low > x0)
        dl->AddRectFilled(bar_p0, ImVec2(x_low, bar_p1.y), cfg.col_green, cfg.bar_round);
    if (x_high > x_low)
        dl->AddRectFilled(ImVec2(x_low, bar_p0.y), ImVec2(x_high, bar_p1.y), cfg.col_yellow, cfg.bar_round);
    if (x1 > x_high)
        dl->AddRectFilled(ImVec2(x_high, bar_p0.y), bar_p1, cfg.col_red, cfg.bar_round);

    // Rim
    dl->AddRect(bar_p0, bar_p1, cfg.col_rim, cfg.bar_round, 0, 1.0f);

    // Optional scale labels (e.g., only max on the right by default)
    auto fmt = cfg.format_ohms ? cfg.format_ohms : default_format_ohms;
    if (cfg.show_threshold_labels)
    {
        char buf_max[32];
        fmt(buf_max, sizeof(buf_max), max_range);
        ImVec2 t_max = ImGui::CalcTextSize(buf_max);
        const float y_top = cursor.y;  // top row
        dl->AddText(ImVec2(x1 - t_max.x, y_top), cfg.col_text, buf_max);
        // If you want a "0" at left, uncomment:
        // dl->AddText(ImVec2(x0, y_top), cfg.col_text, "0");
    }

    // Cursor lines
    const float y0_line = y_bar - dcfg.cursor_overhang;
    const float y1_line = y_bar + bar_h + dcfg.cursor_overhang;
    dl->AddLine(ImVec2(x_low, y0_line), ImVec2(x_low, y1_line), dcfg.cursor_color, dcfg.cursor_thickness);
    dl->AddLine(ImVec2(x_high, y0_line), ImVec2(x_high, y1_line), dcfg.cursor_color, dcfg.cursor_thickness);

    // --- Interaction (drag) ---
    bool changed = false;
    if (dcfg.draggable && (out_low || out_high))
    {
        ImGuiStorage* storage = ImGui::GetStateStorage();
        const ImGuiID key_active = storage_key(id_label, "#active_cursor");  // -1 none, 0=low, 1=high
        int active_cursor = storage->GetInt(key_active, -1);

        // Click → choose nearest cursor within hit slop
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            float mx = ImGui::GetIO().MousePos.x;
            float dx_l = fabsf(mx - x_low);
            float dx_h = fabsf(mx - x_high);
            if (dx_l <= dcfg.hit_slop_px || dx_h <= dcfg.hit_slop_px)
            {
                active_cursor = (dx_l <= dx_h) ? 0 : 1;
                storage->SetInt(key_active, active_cursor);
            }
            else
            {
                storage->SetInt(key_active, -1);
                active_cursor = -1;
            }
        }

        // Drag active cursor
        if (active_cursor != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left) && (hovered || active))
        {
            float mx = ImGui::GetIO().MousePos.x;
            float val = x_to_ohms(mx);

            if (active_cursor == 0 && out_low)
            {
                float max_low = (out_high ? (*out_high - dcfg.min_gap_ohms) : (high_ohms - dcfg.min_gap_ohms));
                float new_low = clampf(val, 0.0f, std::max(0.0f, max_low));
                if (new_low != *out_low)
                {
                    *out_low = new_low;
                    changed = true;
                }
            }
            else if (active_cursor == 1 && out_high)
            {
                float min_high = (out_low ? (*out_low + dcfg.min_gap_ohms) : (low_ohms + dcfg.min_gap_ohms));
                float new_high = clampf(val, std::max(0.0f, min_high), max_range);
                if (new_high != *out_high)
                {
                    *out_high = new_high;
                    changed = true;
                }
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            storage->SetInt(key_active, -1);
        }
    }

    // --- Labels above each cursor (live values) ---
    {
        char left_buf[48], right_buf[48];
        fmt(left_buf, sizeof(left_buf), low_ohms);
        fmt(right_buf, sizeof(right_buf), high_ohms);

        ImVec2 t_left = ImGui::CalcTextSize(left_buf);
        ImVec2 t_right = ImGui::CalcTextSize(right_buf);
        float y_label = cursor.y;  // top row (above bar)

        // Avoid overlap: if labels collide, push left/right
        float min_gap_px = 6.0f;
        float left_x = x_low - t_left.x * 0.5f;
        float right_x = x_high - t_right.x * 0.5f;

        // panel edges
        float x0 = cursor.x + cfg.side_pad;
        float x1 = cursor.x + avail.x - cfg.side_pad;

        // Clamp to panel edges
        left_x = clampf(left_x, x0, x1 - t_left.x);
        right_x = clampf(right_x, x0, x1 - t_right.x);

        if (right_x < left_x + t_left.x + min_gap_px)
        {
            // Push them apart symmetrically
            float overlap = (left_x + t_left.x + min_gap_px) - right_x;
            left_x = clampf(left_x - overlap * 0.5f, x0, x1 - t_left.x);
            right_x = clampf(right_x + overlap * 0.5f, x0, x1 - t_right.x);
        }

        dl->AddText(ImVec2(left_x, y_label), cfg.col_text, left_buf);
        dl->AddText(ImVec2(right_x, y_label), cfg.col_text, right_buf);
    }

    return changed;
}

}  // namespace elda::ui
