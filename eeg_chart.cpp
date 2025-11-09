// =============================================================================
// OPTIMIZED: Scan phase optimizations
// - Remove floor() calls (use direct time comparisons)
// - Reserve capacity (avoid reallocations)
// - Better cache locality (early exit)
// =============================================================================

#include "eeg_chart.h"
#include "imgui.h"
#include "implot.h"

#include <numeric>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <chrono>

static constexpr float kLabelInsetPx = 20.0f;
static constexpr float kLabelSpacingPixels = 50.0f;

static const ImVec4 kLabelColorNormal    = ImVec4(0.65f, 0.68f, 0.72f, 1.0f);
static const ImVec4 kLabelColorHighlight = ImVec4(0.30f, 0.90f, 0.95f, 1.0f);

struct PerfScope {
    std::chrono::steady_clock::time_point t0;
    double* out_ms;
    PerfScope(double* out=nullptr) : t0(std::chrono::steady_clock::now()), out_ms(out) {}
    ~PerfScope() {
        if (!out_ms) return;
        const double ms = std::chrono::duration<double,std::milli>(
            std::chrono::steady_clock::now() - t0).count();
        *out_ms += ms;
    }
};

struct FrameStats {
    double total_ms        = 0.0;
    double label_width_ms  = 0.0;
    double plotting_ms     = 0.0;
    double scan_ms         = 0.0;
    double plot_lines_ms   = 0.0;
    double wiper_ms        = 0.0;
    double labels_ms       = 0.0;
    int    points_rendered = 0;
    int    points_scanned  = 0;
    int    points_skipped  = 0;

    static int counter;
    static constexpr int kLogEvery = 60;

    void Log() {
        if (++counter >= kLogEvery) {
            std::printf("\n=== EEG CHART PERFORMANCE ===\n");
            std::printf("  Total Frame:     %.3f ms\n", total_ms);
            std::printf("  Label Width:     %.3f ms\n", label_width_ms);
            std::printf("  Plotting Total:  %.3f ms\n", plotting_ms);
            std::printf("    Build (scan):  %.3f ms\n", scan_ms);
            std::printf("    Plot lines:    %.3f ms\n", plot_lines_ms);
            std::printf("    Wiper:         %.3f ms\n", wiper_ms);
            std::printf("  Draw Labels:     %.3f ms\n", labels_ms);
            std::printf("  Samples Scanned: %d\n", points_scanned);
            std::printf("  Samples Skipped: %d (%.1f%%)\n", points_skipped,
                       points_scanned > 0 ? 100.0 * points_skipped / points_scanned : 0.0);
            std::printf("  Points Rendered: %d\n", points_rendered);
            std::printf("  FPS:             %.1f\n", ImGui::GetIO().Framerate);
            std::printf("==============================\n\n");
            counter = 0;
        }
    }
};
int FrameStats::counter = 0;

struct PlotBuffers {
    std::vector<float> xsPrev, ysPrev, xsCur, ysCur;

    PlotBuffers() {
        const int maxSize = BUFFER_SIZE;
        xsPrev.reserve(maxSize);
        ysPrev.reserve(maxSize);
        xsCur.reserve(maxSize);
        ysCur.reserve(maxSize);
    }

    void clear() {
        xsPrev.clear();
        ysPrev.clear();
        xsCur.clear();
        ysCur.clear();
    }
};

void DrawChart(AppState& st) {
    FrameStats fs;
    static PlotBuffers plotBuffers;

    {
        PerfScope _total(&fs.total_ms);

        const double rowHeight    = std::max(1.0, 1.2 * (double)st.ampPPuV());
        const int    visibleCount = CHANNELS;
        const double yTop         = rowHeight * (visibleCount + 1);

        {
            PerfScope _lbl(&fs.label_width_ms);
            float maxLabelPx = 0.0f;
            for (int v = 0; v < visibleCount; ++v) {
                char lbl[16];
                std::snprintf(lbl, sizeof(lbl), "ch%02d", v + 1);
                maxLabelPx = std::max(maxLabelPx, ImGui::CalcTextSize(lbl).x);
            }
            (void)maxLabelPx;
        }

        ImGui::BeginChild("##eegchild", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

        const double windowSec = st.windowSec();
        const float plotWidthPixels = ImGui::GetContentRegionAvail().x;
        const double leftPaddingTime = (kLabelSpacingPixels / plotWidthPixels) * windowSec;

        const double xMin = -leftPaddingTime;
        const double xMax = windowSec;

        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
        ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(2, 2));
        ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize, ImVec2(0, 0));
        ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize, ImVec2(0, 0));

        ImPlotFlags plotFlags = ImPlotFlags_NoLegend | ImPlotFlags_NoMenus
                              | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoTitle
                              | ImPlotFlags_NoMouseText;

        ImPlotAxisFlags axFlags = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels
                                | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_Lock
                                | ImPlotAxisFlags_NoHighlight;

        if (ImPlot::BeginPlot("##sweep", ImVec2(-1, -1), plotFlags)) {
            ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, yTop, ImGuiCond_Always);
            ImPlot::SetupAxis(ImAxis_X1, nullptr, axFlags);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr, axFlags);
            ImPlot::SetupFinish();

            PerfScope _plotting(&fs.plotting_ms);

            const double smoothedCursor = st.playheadSeconds;
            const double cycleCur   = std::floor(smoothedCursor / windowSec);
            const double cycleStart = cycleCur * windowSec;
            const double cursorX    = smoothedCursor - cycleStart;
            const double eps        = 0.5 / SAMPLE_RATE_HZ;

            // OPTIMIZATION 1: Pre-calculate cycle boundaries (avoid floor() in loop)
            const double prevCycleStart = (cycleCur - 1.0) * windowSec;
            const double prevCycleEnd   = cycleCur * windowSec;
            const double curCycleStart  = cycleCur * windowSec;
            const double curCycleEnd    = (cycleCur + 1.0) * windowSec;
            const double cursorAbsTime  = curCycleStart + cursorX;

            int hoveredChannel = -1;
            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                for (int v = 0; v < visibleCount; ++v) {
                    const double yBase = rowHeight * (visibleCount - v);
                    const double yMin = yBase - rowHeight * 0.5;
                    const double yMax = yBase + rowHeight * 0.5;
                    if (mouse.y >= yMin && mouse.y <= yMax) {
                        hoveredChannel = v;
                        break;
                    }
                }
            }

            const int N = st.ring.size();

            if (N > 0) {
                for (int v = 0; v < visibleCount; ++v) {
                    const int c = v;
                    const double yBase = rowHeight * (visibleCount - v);

                    plotBuffers.clear();

                    // OPTIMIZATION 2: Reserve capacity (avoid reallocations)
                    const int estimatedPoints = (int)(windowSec * SAMPLE_RATE_HZ * 1.1);
                    plotBuffers.xsCur.reserve(estimatedPoints);
                    plotBuffers.ysCur.reserve(estimatedPoints);
                    plotBuffers.xsPrev.reserve(estimatedPoints);
                    plotBuffers.ysPrev.reserve(estimatedPoints);

                    {
                        PerfScope _scan(&fs.scan_ms);

                        const int startIdx = st.ring.filled ? st.ring.write : 0;
                        const int totalSamples = st.ring.filled ? BUFFER_SIZE : st.ring.write;

                        for (int offset = 0; offset < totalSamples; ++offset) {
                            const int i = (startIdx + offset) % BUFFER_SIZE;
                            const double t = st.ring.tAbs[i];

                            fs.points_scanned++;

                            // OPTIMIZATION 3: Early exit when past visible range
                            if (t < prevCycleStart) {
                                fs.points_skipped++;
                                continue;
                            }

                            if (t > curCycleEnd) {
                                fs.points_skipped += (totalSamples - offset);
                                break; // All remaining samples are beyond visible range
                            }

                            const double val = yBase + st.gainMul() * st.ring.data[c][i];

                            // OPTIMIZATION 1: Direct time comparisons (no floor())
                            if (t >= curCycleStart && t < curCycleEnd) {
                                // Current cycle
                                if (t < cursorAbsTime - eps) {
                                    plotBuffers.xsCur.push_back((float)(t - curCycleStart));
                                    plotBuffers.ysCur.push_back((float)val);
                                }
                            }
                            else if (t >= prevCycleStart && t < prevCycleEnd) {
                                // Previous cycle
                                const float rx = (float)(t - prevCycleStart);
                                if (rx >= cursorX - eps) {
                                    plotBuffers.xsPrev.push_back(rx);
                                    plotBuffers.ysPrev.push_back((float)val);
                                }
                            }
                        }
                    }

                    {
                        PerfScope _plotLines(&fs.plot_lines_ms);
                        char idPrev[32], idCur[32];
                        std::snprintf(idPrev, sizeof(idPrev), "##prev_ch%02d", c + 1);
                        std::snprintf(idCur, sizeof(idCur), "Ch%02d", c + 1);

                        ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                        if (!plotBuffers.xsPrev.empty()) {
                            ImPlot::PlotLine(idPrev, plotBuffers.xsPrev.data(), plotBuffers.ysPrev.data(), (int)plotBuffers.xsPrev.size());
                            fs.points_rendered += (int)plotBuffers.xsPrev.size();
                        }

                        ImPlot::SetNextLineStyle(ImVec4(0.10f, 0.80f, 0.95f, 1.0f), 1.0f);
                        if (!plotBuffers.xsCur.empty()) {
                            ImPlot::PlotLine(idCur, plotBuffers.xsCur.data(), plotBuffers.ysCur.data(), (int)plotBuffers.xsCur.size());
                            fs.points_rendered += (int)plotBuffers.xsCur.size();
                        }
                    }
                }

                {
                    PerfScope _wiper(&fs.wiper_ms);
                    ImDrawList* dl = ImPlot::GetPlotDrawList();
                    ImVec2 ppos = ImPlot::GetPlotPos();
                    ImVec2 psz = ImPlot::GetPlotSize();

                    ImVec2 pc = ImPlot::PlotToPixels(ImPlotPoint(cursorX, 0.0));
                    int cx = (int)std::floor(pc.x);

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
            }

            {
                PerfScope _lbls(&fs.labels_ms);
                const ImPlotRect lim = ImPlot::GetPlotLimits();
                const double xLeft = lim.X.Min;

                for (int v = 0; v < visibleCount; ++v) {
                    const double yBase = rowHeight * (visibleCount - v);
                    char label[16];
                    std::snprintf(label, sizeof(label), "ch%02d", v + 1);

                    ImVec4 labelColor = (v == hoveredChannel) ? kLabelColorHighlight : kLabelColorNormal;

                    ImPlot::PushStyleColor(ImPlotCol_InlayText, labelColor);
                    ImPlot::PlotText(label, xLeft, yBase, ImVec2(+kLabelInsetPx, 0.0f), ImPlotTextFlags_None);
                    ImPlot::PopStyleColor();
                }
            }

            ImPlot::EndPlot();
        }

        ImPlot::PopStyleVar(4);
        ImGui::EndChild();
    }

    fs.Log();
}