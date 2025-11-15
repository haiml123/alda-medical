// chart.cpp — Fixed time base, fixed gain (px/µV), pixel-locked Y BEFORE any locking calls
#include "chart.h"
#include "imgui.h"
#include "implot.h"
#include "models/channel.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static constexpr float k_left_label_inset_px  = 20.0f;
static constexpr float k_left_spacing_px      = 56.0f;
static constexpr float k_top_pad_px           = 8.0f;
static constexpr float k_preferred_row_px     = 120.0f;
static constexpr float k_min_row_px           = 8.0f;
static constexpr int   k_wiper_width_px       = 10;

static const ImVec4 k_label_color_normal = ImVec4(0.72f, 0.76f, 0.80f, 1.0f);

static ImVec4 parse_hex_color(const std::string& hex, const ImVec4& fallback) {
    auto hv = [](char c)->int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };
    if (hex.empty()) return fallback;
    const char* s = hex.c_str();
    if (*s == '#') ++s;
    const size_t n = std::strlen(s);
    auto to01 = [&](int hi, int lo)->float { return float((hi<<4)|lo)/255.0f; };
    float r = fallback.x, g = fallback.y, b = fallback.z, a = fallback.w;
    if (n == 3) {
        r = hv(s[0]) / 15.0f;
        g = hv(s[1]) / 15.0f;
        b = hv(s[2]) / 15.0f;
        a = 1.0f;
    } else if (n == 6 || n == 8) {
        r = to01(hv(s[0]), hv(s[1]));
        g = to01(hv(s[2]), hv(s[3]));
        b = to01(hv(s[4]), hv(s[5]));
        a = (n == 8) ? to01(hv(s[6]), hv(s[7])) : 1.0f;
    }
    return ImVec4(r, g, b, a);
}

struct PlotBuffers {
    std::vector<float> xs_prev;
    std::vector<float> ys_prev;
    std::vector<float> xs_cur;
    std::vector<float> ys_cur;

    void clear() {
        xs_prev.clear();
        ys_prev.clear();
        xs_cur.clear();
        ys_cur.clear();
    }

    void reserve(int n) {
        xs_prev.reserve(n);
        ys_prev.reserve(n);
        xs_cur.reserve(n);
        ys_cur.reserve(n);
    }
};

static inline int resolve_channel_index(
    int v,
    const std::vector<const elda::models::Channel*>& selected,
    const elda::ChartData& data)
{
    int c = v;
    const elda::models::Channel* meta =
        (v < static_cast<int>(selected.size())) ? selected[v] : nullptr;

    if (meta &&
        meta->amplifier_channel >= 0 &&
        meta->amplifier_channel < static_cast<int>(data.ring.data.size())) {
        c = meta->amplifier_channel;
    }
    return c;
}

void draw_chart(const elda::ChartData& data,
                const std::vector<const elda::models::Channel*>& selected_channels)
{
    static PlotBuffers plot_buffers;

    const bool use_selected  = !selected_channels.empty();
    const int  total_channels = data.num_channels;
    const int  visible_count  = use_selected
                                ? static_cast<int>(selected_channels.size())
                                : total_channels;
    const int  rows           = std::max(1, visible_count);

    ImGui::BeginChild("##eegchild", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

    // Fixed time base
    const double window_sec     = data.window_seconds;
    const float  avail_width    = ImGui::GetContentRegionAvail().x;
    const float  avail_height   = ImGui::GetContentRegionAvail().y;
    const double left_padding_time = (k_left_spacing_px / std::max(1.0f, avail_width)) * window_sec;
    const double x_min          = -left_padding_time;
    const double x_max          = window_sec;

    // Style
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding,     ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding,    ImVec2(2, 2));
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize,  0.0f);

    ImPlotFlags plot_flags = ImPlotFlags_NoLegend | ImPlotFlags_NoMenus |
                             ImPlotFlags_NoBoxSelect | ImPlotFlags_NoTitle |
                             ImPlotFlags_NoMouseText;

    ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoLabel |
                                 ImPlotAxisFlags_NoTickLabels |
                                 ImPlotAxisFlags_NoTickMarks |
                                 ImPlotAxisFlags_Lock |
                                 ImPlotAxisFlags_NoHighlight;

    if (ImPlot::BeginPlot("##sweep", ImVec2(-1, -1), plot_flags)) {
        // --------- ALL Setup* happen here, BEFORE any plotting/locking calls ---------
        ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);
        // Pixel-locked Y using ImGui's available height (avoids calling GetPlotSize() pre-setup)
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, static_cast<double>(std::max(1.0f, avail_height)),
                                ImGuiCond_Always);
        ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_flags);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_flags);
        ImPlot::SetupFinish(); // ---- lock: no more Setup* below this line ----

        // Row spacing (auto-pack to fit)
        const double row_height_px = std::min<double>(
            k_preferred_row_px,
            std::max<double>(k_min_row_px,
                             (avail_height - k_top_pad_px) / std::max(1, rows))
        );

        // Sweep timing
        const double smoothed_cursor = data.playhead_seconds;
        const double cycle_cur       = std::floor(smoothed_cursor / window_sec);
        const double cursor_x        = smoothed_cursor - cycle_cur * window_sec;
        const double eps             = 0.5 / std::max(1, data.sample_rate_hz);

        const double prev_cycle_start = (cycle_cur - 1.0) * window_sec;
        const double prev_cycle_end   = cycle_cur * window_sec;
        const double cur_cycle_start  = cycle_cur * window_sec;
        const double cur_cycle_end    = (cycle_cur + 1.0) * window_sec;
        const double cursor_abs_time  = cur_cycle_start + cursor_x;

        const int estimated_points = static_cast<int>(window_sec * data.sample_rate_hz * 1.1);

        // Plot channels (fixed gain px/µV)
        for (int row = 0; row < rows; ++row) {
            const int visible_index = row;
            const int channel_index = use_selected
                                      ? resolve_channel_index(visible_index,
                                                              selected_channels,
                                                              data)
                                      : visible_index;

            const double y_base = k_top_pad_px + (row + 0.5) * row_height_px;

            plot_buffers.clear();
            plot_buffers.reserve(estimated_points);

            const int start_index   = data.ring.filled ? data.ring.write : 0;
            const int total_samples = data.ring.filled ? data.buffer_size : data.ring.write;

            for (int offset = 0; offset < total_samples; ++offset) {
                const int i = (start_index + offset) % data.buffer_size;
                const double t = data.ring.t_abs[i];
                if (t < prev_cycle_start) continue;
                if (t > cur_cycle_end)    break;

                // FIXED gain: sample is µV; multiply by px/µV
                const double y =
                    y_base + data.gain_multiplier * data.ring.data[channel_index][i];

                if (t >= cur_cycle_start && t < cur_cycle_end) {
                    if (t < cursor_abs_time - eps) {
                        plot_buffers.xs_cur.push_back(static_cast<float>(t - cur_cycle_start));
                        plot_buffers.ys_cur.push_back(static_cast<float>(y));
                    }
                } else if (t >= prev_cycle_start && t < prev_cycle_end) {
                    const float relative_x = static_cast<float>(t - prev_cycle_start);
                    if (relative_x >= cursor_x - eps) {
                        plot_buffers.xs_prev.push_back(relative_x);
                        plot_buffers.ys_prev.push_back(static_cast<float>(y));
                    }
                }
            }

            const elda::models::Channel* meta =
                (use_selected && row < static_cast<int>(selected_channels.size()))
                    ? selected_channels[row]
                    : nullptr;

            const ImVec4 k_cyan = ImVec4(0.10f, 0.80f, 0.95f, 1.0f);
            ImVec4 line_color = (meta && !meta->color.empty())
                                ? parse_hex_color(meta->color, k_cyan)
                                : k_cyan;

            const char* base_name =
                (meta && !meta->name.empty()) ? meta->name.c_str() : nullptr;

            char id_prev[64];
            char id_cur[64];
            if (base_name) {
                std::snprintf(id_prev, sizeof(id_prev), "##prev_%s", base_name);
                std::snprintf(id_cur,  sizeof(id_cur),  "%s",       base_name);
            } else {
                std::snprintf(id_prev, sizeof(id_prev), "##prev_ch%02d", row + 1);
                std::snprintf(id_cur,  sizeof(id_cur),  "Ch%02d",        row + 1);
            }

            ImPlot::SetNextLineStyle(line_color, 1.0f);
            if (!plot_buffers.xs_prev.empty()) {
                ImPlot::PlotLine(id_prev,
                                 plot_buffers.xs_prev.data(),
                                 plot_buffers.ys_prev.data(),
                                 static_cast<int>(plot_buffers.xs_prev.size()));
            }

            ImPlot::SetNextLineStyle(line_color, 1.0f);
            if (!plot_buffers.xs_cur.empty()) {
                ImPlot::PlotLine(id_cur,
                                 plot_buffers.xs_cur.data(),
                                 plot_buffers.ys_cur.data(),
                                 static_cast<int>(plot_buffers.xs_cur.size()));
            }
        }

        // Wiper (OK to use plot queries; NO Setup* calls below)
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();
        ImVec2 plot_pos = ImPlot::GetPlotPos();
        ImVec2 plot_size = ImPlot::GetPlotSize();
        ImVec2 cursor_pixels = ImPlot::PlotToPixels(ImPlotPoint(cursor_x, 0.0));
        int cursor_pixel_x = static_cast<int>(std::floor(cursor_pixels.x));
        int x0 = static_cast<int>(std::max(plot_pos.x,
                                           static_cast<float>(cursor_pixel_x)));
        int x1 = static_cast<int>(std::min(plot_pos.x + plot_size.x,
                                           static_cast<float>(cursor_pixel_x + k_wiper_width_px)));
        if (x1 > x0) {
            ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
            if (bg.w == 0.0f) {
                bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            }
            bg.w = 1.0f;
            draw_list->AddRectFilled(ImVec2(static_cast<float>(x0), plot_pos.y),
                                     ImVec2(static_cast<float>(x1),
                                            plot_pos.y + plot_size.y),
                                     ImGui::GetColorU32(bg));
        }

        // Left labels
        const ImPlotRect limits = ImPlot::GetPlotLimits();
        const double x_left = limits.X.Min;
        for (int row = 0; row < rows; ++row) {
            const elda::models::Channel* meta =
                (use_selected && row < static_cast<int>(selected_channels.size()))
                    ? selected_channels[row]
                    : nullptr;

            char label[64];
            if (meta && !meta->name.empty()) {
                std::snprintf(label, sizeof(label), "%s", meta->name.c_str());
            } else {
                std::snprintf(label, sizeof(label), "ch%02d", row + 1);
            }

            const double row_height_for_label = std::min<double>(
                k_preferred_row_px,
                std::max<double>(k_min_row_px,
                                 (avail_height - k_top_pad_px) / std::max(1, rows))
            );
            const double y_base = k_top_pad_px +
                                  (row + 0.5) * row_height_for_label;

            ImPlot::PushStyleColor(ImPlotCol_InlayText, k_label_color_normal);
            ImPlot::PlotText(label,
                             x_left,
                             y_base,
                             ImVec2(+k_left_label_inset_px, 0.0f),
                             ImPlotTextFlags_None);
            ImPlot::PopStyleColor();
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar(5);
    ImGui::EndChild();
}
