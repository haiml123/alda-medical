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

// Opaque "wiper" width (px)
static constexpr int kWiperPx = 18;

// Small per-label nudge rightwards so text isn't on the frame
static constexpr float kLabelInsetPx = 25.0f;

// Label width in plot coordinates (adjust based on your time scale)
static constexpr double kLabelWidthPlotUnits = 0.3;  // ~0.3 seconds for labels

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

    // Container that fills the remaining space (no scrollbars)
    ImGui::BeginChild("plot", ImVec2(0, 0), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Subtle style + dynamic left padding so labels fit neatly
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding,  ImVec2(4.0f, 2.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_LegendPadding,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize, 0.0f);
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, ImVec4(0,0,0,0));
    // Transparent everything + no border/frame
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,       ImVec4(0,0,0,0)); // frame behind axes
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,        ImVec4(0,0,0,0)); // plot area
    ImPlot::PushStyleColor(ImPlotCol_AxisBg,        ImVec4(0,0,0,0)); // axis background (the gray strip)
    ImPlot::PushStyleColor(ImPlotCol_AxisBgHovered, ImVec4(0,0,0,0)); // also transparent on hover
    ImPlot::PushStyleColor(ImPlotCol_AxisBgActive,  ImVec4(0,0,0,0)); // and when active
    ImPlot::PushStyleVar  (ImPlotStyleVar_PlotBorderSize, 0.0f);

    ImVec2 plot_size = ImGui::GetContentRegionAvail();
    if (plot_size.x < 100.0f) plot_size.x = 100.0f;

    if (ImPlot::BeginPlot("##eeg", plot_size,
                          ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect)) {

        // Axes - shift X axis to start after label area
        const double xMin = -kLabelWidthPlotUnits;  // Start before 0 to make room for labels
        const double xMax = double(st.windowSec());
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)", ImPlotAxisFlags_NoGridLines);  // Disable grid lines
        ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);

        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines);  // Disable grid lines
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, yTop, ImGuiCond_Always);

        // Sweep timing
        const double cycleCur   = std::floor(st.ring.now / st.windowSec());
        const double cycleStart = cycleCur * st.windowSec();
        const double cursorX    = st.ring.now - cycleStart;        // [0, windowSec)
        const double eps        = 0.5 / SAMPLE_RATE_HZ;            // half-sample hysteresis

        // Draw channels (erase semantics across the cursor)
        const int N = st.ring.size();
        std::vector<float> xsPrev, ysPrev, xsCur, ysCur;

        if (N > 0) {
            for (int v = 0; v < visibleCount; ++v) {
                const int    c     = v;                           // channel index
                const double yBase = rowHeight * (visibleCount - v);  // REVERSED: higher v = higher y

                xsPrev.clear(); ysPrev.clear();
                xsCur.clear();  ysCur.clear();
                xsPrev.reserve(N); ysPrev.reserve(N);
                xsCur.reserve(N);  ysCur.reserve(N);

                for (int i = 0; i < N; ++i) {
                    const double t   = st.ring.tAbs[i];
                    const double cyc = std::floor(t / st.windowSec());
                    const double rx  = t - cyc * st.windowSec();
                    const double val = yBase + st.gainMul() * st.ring.data[c][i];

                    if (cyc == cycleCur) {
                        if (rx < cursorX - eps) { xsCur.push_back((float)rx); ysCur.push_back((float)val); }
                    }
                    else if (cyc == cycleCur - 1.0) {
                        if (rx >= cursorX - eps) { xsPrev.push_back((float)rx); ysPrev.push_back((float)val); }
                    }
                }

                sort_by_x(xsPrev, ysPrev);
                sort_by_x(xsCur,  ysCur);

                char idPrev[32], idCur[32];
                std::snprintf(idPrev, sizeof(idPrev), "##prev_ch%02d", c + 1);
                std::snprintf(idCur,  sizeof(idCur),  "Ch%02d",        c + 1);

                ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                if (!xsPrev.empty()) ImPlot::PlotLine(idPrev, xsPrev.data(), ysPrev.data(), (int)xsPrev.size());

                ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                if (!xsCur.empty())  ImPlot::PlotLine(idCur,  xsCur.data(),  ysCur.data(),  (int)xsCur.size());
            }

            // Opaque wiper ahead of the cursor (plot background color)
            ImDrawList* dl   = ImPlot::GetPlotDrawList();
            ImVec2      ppos = ImPlot::GetPlotPos();
            ImVec2      psz  = ImPlot::GetPlotSize();

            ImVec2 pc = ImPlot::PlotToPixels(ImPlotPoint(cursorX, 0.0));
            int cx = (int)std::floor(pc.x);
            int x0 = std::max((int)ppos.x, cx);
            int x1 = std::min((int)(ppos.x + psz.x), cx + kWiperPx);
            if (x1 > x0) {
                // prefer ChildBg if your parent window uses it; otherwise WindowBg
                ImVec4 mainBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
                if (mainBg.w == 0.0f) mainBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                mainBg.w = 1.0f; // fully opaque so it truly wipes
                dl->AddRectFilled(ImVec2((float)x0, ppos.y), ImVec2((float)x1, ppos.y + psz.y),
                                  ImGui::GetColorU32(mainBg));
            }
        }

        // ---------------- Left-side channel labels (inside plot) ----------------
        const ImPlotRect lim  = ImPlot::GetPlotLimits();
        const double     xLeft = lim.X.Min;  // This will now be negative

        for (int v = 0; v < visibleCount; ++v) {
            const double yBase = rowHeight * (visibleCount - v);  // REVERSED: same as waveform
            char label[16];
            std::snprintf(label, sizeof(label), "ch%02d", v + 1);

            // Position label at yBase with NO vertical offset
            ImPlot::PlotText(label,
                             xLeft, yBase,
                             ImVec2(+kLabelInsetPx, 0.0f),
                             ImPlotTextFlags_None);
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar(0); // PlotPadding, LabelPadding, MinorGridSize, MajorGridSize
    ImGui::EndChild();
}