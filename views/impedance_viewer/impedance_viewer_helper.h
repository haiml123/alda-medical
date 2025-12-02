#pragma once
#include "imgui.h"
#include <string>

namespace elda::views::impedance_viewer {

    ImU32 channel_color_from_id(const std::string& channel_id);

    // Cap outline + grid (ears + nose, thin lines)
    void draw_cap_outline(ImDrawList* dl, const ImVec2& center, float radius);
    void draw_cap_grid(ImDrawList* dl, const ImVec2& center, float radius);

    // Cap-relative coordinate transforms
    ImVec2 cap_normalized_to_screen(const ImVec2& center, float radius, float norm_x, float norm_y);
    ImVec2 screen_to_cap_normalized_clamped(const ImVec2& center, float radius, const ImVec2& p);

    // Hit test
    bool point_in_circle(const ImVec2& p, const ImVec2& c, float r);

} // namespace elda::impedance_viewer::helper
