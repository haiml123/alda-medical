#pragma once
#include "imgui.h"

namespace elda::ui {

// Segment thresholds (still used for overall max scale; color now follows cursors)
struct ImpedanceRanges {
    float ok_max;     // not used for color anymore; kept for compatibility
    float warn_max;   // not used for color anymore; kept for compatibility
    float max_range;  // end of scale (full width)
};

struct ImpedanceRangeConfig {
    // Size
    float bar_height       = 16.0f;
    float bar_round        = 4.0f;
    float top_labels_gap   = 6.0f;
    float side_pad         = 8.0f;

    // Colors
    ImU32 col_green        = IM_COL32( 64, 200,  90, 255);
    ImU32 col_yellow       = IM_COL32(245, 210,  70, 255);
    ImU32 col_red          = IM_COL32(215,  70,  60, 255);
    ImU32 col_rim          = IM_COL32(160, 160, 175, 255);
    ImU32 col_text         = IM_COL32(230, 230, 235, 255);

    // Optional scale labels (e.g., show only max on the right)
    bool  show_threshold_labels = false;

    // Optional label formatter (defaults to K units).
    void (*format_ohms)(char*, int, float) = nullptr;
};

struct DualCursorConfig {
    // Cursor look/feel
    ImU32 cursor_color      = IM_COL32(0, 0, 0, 255); // black
    float cursor_thickness  = 2.0f;
    float cursor_overhang   = 6.0f;  // pixels above/below the bar
    float hit_slop_px       = 10.0f; // how easy it is to grab the line

    // Behavior
    bool  draggable         = true;
    float min_gap_ohms      = 500.0f; // prevent overlap (>= this distance)
};

// Two-cursor range widget with dynamic coloring and live value labels above cursors.
// Returns true if either cursor changed this frame.
bool draw_impedance_range_dual(const char* id_label,
                               float low_ohms,
                               float high_ohms,
                               const ImpedanceRanges& ranges,
                               const ImpedanceRangeConfig& cfg = {},
                               const DualCursorConfig& dcfg = {},
                               float* out_low = nullptr,
                               float* out_high = nullptr);

} // namespace elda::ui
