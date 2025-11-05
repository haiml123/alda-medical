#include "eeg_chart.h"
#include <numeric>

static void sort_by_x(std::vector<float>& xs, std::vector<float>& ys) {
    if (xs.size() < 2) return;
    bool sorted = true;
    for (size_t k=1;k<xs.size();++k) if (xs[k] < xs[k-1]) { sorted=false; break; }
    if (!sorted) {
        std::vector<size_t> idx(xs.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return xs[a] < xs[b]; });
        std::vector<float> xs2(xs.size()), ys2(ys.size());
        for (size_t k=0;k<idx.size();++k){ xs2[k]=xs[idx[k]]; ys2[k]=ys[idx[k]]; }
        xs.swap(xs2); ys.swap(ys2);
    }
}

void DrawChart(AppState& st) {
    // compute row height from amplitude (even spacing; 20% headroom)
    const double rowHeight = std::max(1.0, 1.2 * (double)st.ampPPuV());
    const int visibleCount = CHANNELS;
    const double yTop = rowHeight * (visibleCount + 1);

    ImGui::BeginChild("plot", ImVec2(0, 0), false,
                      ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);

    if (ImPlot::BeginPlot("##eeg", ImVec2(-1, -1),
                          ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect)) {

        const double xMin = 0.0, xMax = double(st.windowSec());
        ImPlot::SetupAxis(ImAxis_X1, "Time (s)");
        ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, yTop, ImGuiCond_Always);

        // sweep positions
        const double cycleCur   = std::floor(st.ring.now / st.windowSec());
        const double cycleStart = cycleCur * st.windowSec();
        const double cursorX    = st.ring.now - cycleStart; // [0, windowSec)
        const double eps        = 0.5 / SAMPLE_RATE_HZ;      // half-sample hysteresis

        const int N = st.ring.size();
        std::vector<float> xsPrev, ysPrev, xsCur, ysCur;
        if (N > 0) {
            for (int v = 0; v < visibleCount; ++v) {
                const int c = v;
                const double yBase = rowHeight * (v + 1);
                xsPrev.clear(); ysPrev.clear(); xsCur.clear(); ysCur.clear();
                xsPrev.reserve(N); ysPrev.reserve(N); xsCur.reserve(N); ysCur.reserve(N);

                for (int i=0;i<N;++i) {
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
                std::snprintf(idPrev, sizeof(idPrev), "##prev_ch%02d", c+1);
                std::snprintf(idCur,  sizeof(idCur),  "Ch%02d",        c+1);

                ImPlot::SetNextLineStyle(ImVec4(0.10f,0.80f,0.95f,1.0f), 1.0f);
                if (!xsPrev.empty()) ImPlot::PlotLine(idPrev, xsPrev.data(), ysPrev.data(), (int)xsPrev.size());

                ImPlot::SetNextLineStyle(ImVec4(0.10f,0.80f,0.95f,1.0f), 1.0f);
                if (!xsCur.empty())  ImPlot::PlotLine(idCur,  xsCur.data(),  ysCur.data(),  (int)xsCur.size());
            }

            // Pixel-snapped blanking wiper (opaque plot bg), clamped to plot rect
            const int wipe_px = 18;
            ImDrawList*  dl        = ImPlot::GetPlotDrawList();
            ImVec2       plot_pos  = ImPlot::GetPlotPos();
            ImVec2       plot_size = ImPlot::GetPlotSize();
            float        plot_x0   = plot_pos.x, plot_x1 = plot_pos.x + plot_size.x;
            float        plot_y0   = plot_pos.y, plot_y1 = plot_pos.y + plot_size.y;

            ImVec2 pc = ImPlot::PlotToPixels(ImPlotPoint(cursorX, 0.0));
            int cx = (int)std::floor(pc.x);
            int x0 = std::max((int)plot_x0, cx);
            int x1 = std::min((int)plot_x1, cx + wipe_px);
            if (x1 > x0) {
                ImVec4 bg = ImPlot::GetStyle().Colors[ImPlotCol_PlotBg]; bg.w = 1.0f;
                dl->AddRectFilled(ImVec2((float)x0, plot_y0), ImVec2((float)x1, plot_y1), ImGui::GetColorU32(bg));
            }
        }

        // Left labels
        {
            const ImVec2 plotMin = ImPlot::GetPlotPos();
            ImDrawList* wdl = ImGui::GetWindowDrawList();
            const float pad = 6.0f;
            for (int v=0; v<CHANNELS; ++v) {
                const double yBase = rowHeight * (v + 1);
                ImVec2 py = ImPlot::PlotToPixels(ImPlotPoint(xMin, yBase));
                const char* label = st.chNames[v].c_str();
                ImVec2 ts = ImGui::CalcTextSize(label);
                ImVec2 pos(plotMin.x - pad - ts.x, py.y - ts.y * 0.5f);
                wdl->AddText(ImVec2(pos.x+1,pos.y+1), IM_COL32(0,0,0,180), label);
                wdl->AddText(pos, IM_COL32(200,200,200,255), label);
            }
        }

        ImPlot::EndPlot();
    }
    ImGui::EndChild();
}