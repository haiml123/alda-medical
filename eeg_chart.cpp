// =============================================================================
// FEATURE: Hover Highlight - Change Label Color on Waveform Hover
// =============================================================================
// When mouse hovers over a waveform, the corresponding channel label changes
// to cyan (same color as waveform)
// =============================================================================

#include "eeg_chart.h"

// GUI
#include "imgui.h"
#include "implot.h"

#include <numeric>
#include <algorithm>
#include <cstdio>
#include <cmath>

// ----------------------------- helpers -----------------------------
static void sort_by_x(std::vector<float>& xs, std::vector<float>& ys) {
    if (xs.size() < 2) return;
    bool sorted = true;
    for (size_t k = 1; k < xs.size(); ++k)
        if (xs[k] < xs[k - 1]) { sorted = false; break; }
    if (!sorted) {
        std::vector<size_t> idx(xs.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(),
                         [&](size_t a, size_t b){ return xs[a] < xs[b]; });
        std::vector<float> xs2(xs.size()), ys2(ys.size());
        for (size_t k = 0; k < idx.size(); ++k) {
            xs2[k] = xs[idx[k]];
            ys2[k] = ys[idx[k]];
        }
        xs.swap(xs2); ys.swap(ys2);
    }
}

// Small per-label nudge rightwards so text isn't on the frame
static constexpr float kLabelInsetPx = 25.0f;

// Label spacing (constant visual spacing at all time scales)
static constexpr float kLabelSpacingPixels = 50.0f;

// =============================================================================
// Color constants for labels
// =============================================================================
static const ImVec4 kLabelColorNormal    = ImVec4(0.65f, 0.68f, 0.72f, 1.0f);  // Gray (default)
static const ImVec4 kLabelColorHighlight = ImVec4(0.30f, 0.90f, 0.95f, 1.0f);  // Cyan (waveform color)

// ----------------------------- main -----------------------------
void DrawChart(AppState& st) {
    // Vertical band spacing (plot units); keep consistent with your amplitude scaling
    const double rowHeight    = std::max(1.0, 1.2 * (double)st.ampPPuV());
    const int    visibleCount = CHANNELS;                 // draw all channels
    const double yTop         = rowHeight * (visibleCount + 1);

    // ------------------------------------------------------------
    // Dynamically compute left plot padding from the longest label
    // ------------------------------------------------------------
    float maxLabelPx = 0.0f;
    for (int v = 0; v < visibleCount; ++v) {
        char lbl[16];
        std::snprintf(lbl, sizeof(lbl), "ch%02d", v + 1);
        maxLabelPx = std::max(maxLabelPx, ImGui::CalcTextSize(lbl).x);
    }

    // Container that fills the remaining space
    ImGui::BeginChild("##eegchild", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

    // Calculate pixel-based label padding
    const double windowSec = st.windowSec();
    const float plotWidthPixels = ImGui::GetContentRegionAvail().x;
    const double leftPaddingTime = (kLabelSpacingPixels / plotWidthPixels) * windowSec;

    // Build the plot limits
    const double xMin = -leftPaddingTime;
    const double xMax = windowSec;

    // Style tweaks
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(2, 2));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize, ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize, ImVec2(0, 0));

    // Build the plot
    ImPlotFlags plotFlags = ImPlotFlags_NoLegend
                          | ImPlotFlags_NoMenus
                          | ImPlotFlags_NoBoxSelect
                          | ImPlotFlags_NoTitle
                          | ImPlotFlags_NoMouseText;

    ImPlotAxisFlags axFlags = ImPlotAxisFlags_NoLabel
                            | ImPlotAxisFlags_NoTickLabels
                            | ImPlotAxisFlags_NoTickMarks
                            | ImPlotAxisFlags_Lock
                            | ImPlotAxisFlags_NoHighlight;

    if (ImPlot::BeginPlot("##sweep", ImVec2(-1, -1), plotFlags)) {
        // Set up axes
        ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, yTop, ImGuiCond_Always);

        ImPlot::SetupAxis(ImAxis_X1, nullptr, axFlags);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, axFlags);

        // Lock the view
        ImPlot::SetupFinish();

        // Get smoothed cursor position from AppState
        const double smoothedCursor = st.displayNowSmoothed;

        // Sweep timing
        const double cycleCur   = std::floor(smoothedCursor / windowSec);
        const double cycleStart = cycleCur * windowSec;
        const double cursorX    = smoothedCursor - cycleStart;
        const double eps        = 0.5 / SAMPLE_RATE_HZ;

        // =============================================================================
        // Detect which channel is being hovered
        // =============================================================================
        int hoveredChannel = -1;  // -1 means no hover

        if (ImPlot::IsPlotHovered()) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();

            // Find which channel row the mouse is over
            for (int v = 0; v < visibleCount; ++v) {
                const double yBase = rowHeight * (visibleCount - v);
                const double yMin = yBase - rowHeight * 0.5;
                const double yMax = yBase + rowHeight * 0.5;

                if (mouse.y >= yMin && mouse.y <= yMax) {
                    hoveredChannel = v;  // Store which channel is hovered
                    break;
                }
            }
        }

        // Draw channels (erase semantics across the cursor)
        const int N = st.ring.size();
        std::vector<float> xsPrev, ysPrev, xsCur, ysCur;

        if (N > 0) {
            for (int v = 0; v < visibleCount; ++v) {
                const int    c     = v;
                const double yBase = rowHeight * (visibleCount - v);

                xsPrev.clear(); ysPrev.clear();
                xsCur.clear();  ysCur.clear();
                xsPrev.reserve(N); ysPrev.reserve(N);
                xsCur.reserve(N);  ysCur.reserve(N);

                for (int i = 0; i < N; ++i) {
                    const double t   = st.ring.tAbs[i];
                    const double cyc = std::floor(t / windowSec);
                    const double rx  = t - cyc * windowSec;
                    const double val = yBase + st.gainMul() * st.ring.data[c][i];

                    if (cyc == cycleCur) {
                        if (rx < cursorX - eps) {
                            xsCur.push_back((float)rx);
                            ysCur.push_back((float)val);
                        }
                    }
                    else if (cyc == cycleCur - 1.0) {
                        if (rx >= cursorX - eps) {
                            xsPrev.push_back((float)rx);
                            ysPrev.push_back((float)val);
                        }
                    }
                }

                sort_by_x(xsPrev, ysPrev);
                sort_by_x(xsCur,  ysCur);

                char idPrev[32], idCur[32];
                std::snprintf(idPrev, sizeof(idPrev), "##prev_ch%02d", c + 1);
                std::snprintf(idCur,  sizeof(idCur),  "Ch%02d",        c + 1);

                // Draw waveforms (color unchanged)
                ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                if (!xsPrev.empty()) {
                    ImPlot::PlotLine(idPrev, xsPrev.data(), ysPrev.data(), (int)xsPrev.size());
                }

                ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                if (!xsCur.empty()) {
                    ImPlot::PlotLine(idCur, xsCur.data(), ysCur.data(), (int)xsCur.size());
                }
            }

            // Cursor wiper (FIXED WIDTH)
            ImDrawList* dl   = ImPlot::GetPlotDrawList();
            ImVec2      ppos = ImPlot::GetPlotPos();
            ImVec2      psz  = ImPlot::GetPlotSize();

            ImVec2 pc = ImPlot::PlotToPixels(ImPlotPoint(cursorX, 0.0));
            int cx = (int)std::floor(pc.x);

            // Fixed pixel width - does not scale with window time
            constexpr int wiperWidthPixels = 10;

            int x0 = std::max((int)ppos.x, cx);
            int x1 = std::min((int)(ppos.x + psz.x), cx + wiperWidthPixels);

            if (x1 > x0) {
                ImVec4 mainBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
                if (mainBg.w == 0.0f) {
                    mainBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                }
                mainBg.w = 1.0f;

                dl->AddRectFilled(
                    ImVec2((float)x0, ppos.y),
                    ImVec2((float)x1, ppos.y + psz.y),
                    ImGui::GetColorU32(mainBg)
                );
            }
        }

        // =============================================================================
        // Channel labels with hover highlight
        // =============================================================================
        const ImPlotRect lim  = ImPlot::GetPlotLimits();
        const double     xLeft = lim.X.Min;

        for (int v = 0; v < visibleCount; ++v) {
            const double yBase = rowHeight * (visibleCount - v);
            char label[16];
            std::snprintf(label, sizeof(label), "ch%02d", v + 1);

            // Choose color based on hover state
            ImVec4 labelColor = (v == hoveredChannel) ? kLabelColorHighlight : kLabelColorNormal;

            // Convert ImVec4 color to plot coordinate space for PlotText
            ImPlot::PushStyleColor(ImPlotCol_InlayText, labelColor);

            ImPlot::PlotText(
                label,
                xLeft, yBase,
                ImVec2(+kLabelInsetPx, 0.0f),
                ImPlotTextFlags_None
            );

            ImPlot::PopStyleColor();  // Restore color
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar(4);
    ImGui::EndChild();
}