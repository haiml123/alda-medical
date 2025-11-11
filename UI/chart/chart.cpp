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

static constexpr float kLeftLabelInsetPx  = 20.0f;
static constexpr float kLeftSpacingPx     = 56.0f;
static constexpr float kTopPadPx          = 8.0f;
static constexpr float kPreferredRowPx    = 120.0f;
static constexpr float kMinRowPx          = 8.0f;
static constexpr int   kWiperWidthPx      = 10;

static const ImVec4 kLabelColorNormal = ImVec4(0.72f, 0.76f, 0.80f, 1.0f);

static ImVec4 ParseHexColor(const std::string& hex, const ImVec4& fallback) {
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
    float r=fallback.x,g=fallback.y,b=fallback.z,a=fallback.w;
    if (n==3){ r=hv(s[0])/15.f; g=hv(s[1])/15.f; b=hv(s[2])/15.f; a=1.f; }
    else if (n==6||n==8){
        r=to01(hv(s[0]),hv(s[1])); g=to01(hv(s[2]),hv(s[3]));
        b=to01(hv(s[4]),hv(s[5])); a=(n==8)?to01(hv(s[6]),hv(s[7])):1.f;
    }
    return ImVec4(r,g,b,a);
}

struct PlotBuffers {
    std::vector<float> xsPrev, ysPrev, xsCur, ysCur;
    void clear(){ xsPrev.clear(); ysPrev.clear(); xsCur.clear(); ysCur.clear(); }
    void reserve(int n){ xsPrev.reserve(n); ysPrev.reserve(n); xsCur.reserve(n); ysCur.reserve(n); }
};

static inline int ResolveChannelIndex(
    int v, const std::vector<const elda::models::Channel*>& selected,
    const elda::ChartData& data) {
    int c = v;
    const elda::models::Channel* meta = (v < (int)selected.size()) ? selected[v] : nullptr;
    if (meta && meta->amplifierChannel >= 0 &&
        meta->amplifierChannel < (int)data.ring.data.size())
        c = meta->amplifierChannel;
    return c;
}

void DrawChart(const elda::ChartData& data,
               const std::vector<const elda::models::Channel*>& selectedChannels) {
    static PlotBuffers buf;

    const bool useSelected  = !selectedChannels.empty();
    const int  totalChans   = data.numChannels;
    const int  visibleCount = useSelected ? (int)selectedChannels.size() : totalChans;
    const int  rows         = std::max(1, visibleCount);

    ImGui::BeginChild("##eegchild", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

    // Fixed time base
    const double windowSec       = data.windowSeconds;
    const float  availW_gui      = ImGui::GetContentRegionAvail().x; // safe before setup
    const float  availH_gui      = ImGui::GetContentRegionAvail().y; // we use this for Y limits
    const double leftPaddingTime = (kLeftSpacingPx / std::max(1.0f, availW_gui)) * windowSec;
    const double xMin            = -leftPaddingTime;
    const double xMax            = windowSec;

    // Style
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding,     ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding,    ImVec2(2, 2));
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize,   ImVec2(0, 0));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize,  0.0f);

    ImPlotFlags plotFlags = ImPlotFlags_NoLegend | ImPlotFlags_NoMenus |
                            ImPlotFlags_NoBoxSelect | ImPlotFlags_NoTitle |
                            ImPlotFlags_NoMouseText;

    ImPlotAxisFlags axFlags = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels |
                              ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_Lock |
                              ImPlotAxisFlags_NoHighlight;

    if (ImPlot::BeginPlot("##sweep", ImVec2(-1, -1), plotFlags)) {
        // --------- ALL Setup* happen here, BEFORE any plotting/locking calls ---------
        ImPlot::SetupAxisLimits(ImAxis_X1, xMin, xMax, ImGuiCond_Always);
        // Pixel-locked Y using ImGui's available height (avoids calling GetPlotSize() pre-setup)
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, (double)std::max(1.0f, availH_gui), ImGuiCond_Always);
        ImPlot::SetupAxis(ImAxis_X1, nullptr, axFlags);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, axFlags);
        ImPlot::SetupFinish(); // ---- lock: no more Setup* below this line ----

        // Row spacing (auto-pack to fit)
        const double rowHeightPx = std::min<double>(
            kPreferredRowPx,
            std::max<double>(kMinRowPx, (availH_gui - kTopPadPx) / std::max(1, rows))
        );

        // Sweep timing
        const double smoothedCursor = data.playheadSeconds;
        const double cycleCur       = std::floor(smoothedCursor / windowSec);
        const double cursorX        = smoothedCursor - cycleCur * windowSec;
        const double eps            = 0.5 / std::max(1, data.sampleRateHz);

        const double prevCycleStart = (cycleCur - 1.0) * windowSec;
        const double prevCycleEnd   = cycleCur * windowSec;
        const double curCycleStart  = cycleCur * windowSec;
        const double curCycleEnd    = (cycleCur + 1.0) * windowSec;
        const double cursorAbsTime  = curCycleStart + cursorX;

        const int estPts = (int)(windowSec * data.sampleRateHz * 1.1);

        // Plot channels (fixed gain px/µV)
        for (int row = 0; row < rows; ++row) {
            const int v = row;
            const int c = useSelected ? ResolveChannelIndex(v, selectedChannels, data) : v;

            const double yBase = kTopPadPx + (row + 0.5) * rowHeightPx;

            buf.clear();
            buf.reserve(estPts);

            const int startIdx     = data.ring.filled ? data.ring.write : 0;
            const int totalSamples = data.ring.filled ? data.bufferSize : data.ring.write;

            for (int off = 0; off < totalSamples; ++off) {
                const int i = (startIdx + off) % data.bufferSize;
                const double t = data.ring.tAbs[i];
                if (t < prevCycleStart) continue;
                if (t > curCycleEnd)    break;

                // FIXED gain: sample is µV; multiply by px/µV
                const double y = yBase + data.gainMultiplier * data.ring.data[c][i];

                if (t >= curCycleStart && t < curCycleEnd) {
                    if (t < cursorAbsTime - eps) {
                        buf.xsCur.push_back((float)(t - curCycleStart));
                        buf.ysCur.push_back((float)y);
                    }
                } else if (t >= prevCycleStart && t < prevCycleEnd) {
                    const float rx = (float)(t - prevCycleStart);
                    if (rx >= cursorX - eps) {
                        buf.xsPrev.push_back(rx);
                        buf.ysPrev.push_back((float)y);
                    }
                }
            }

            const elda::models::Channel* meta =
                (useSelected && row < (int)selectedChannels.size()) ? selectedChannels[row] : nullptr;

            const ImVec4 kCyan = ImVec4(0.10f, 0.80f, 0.95f, 1.0f);
            ImVec4 lineColor = (meta && !meta->color.empty())
                               ? ParseHexColor(meta->color, kCyan) : kCyan;

            const char* baseName = (meta && !meta->name.empty()) ? meta->name.c_str() : nullptr;
            char idPrev[64], idCur[64];
            if (baseName) {
                std::snprintf(idPrev, sizeof(idPrev), "##prev_%s", baseName);
                std::snprintf(idCur,  sizeof(idCur),  "%s",       baseName);
            } else {
                std::snprintf(idPrev, sizeof(idPrev), "##prev_ch%02d", row + 1);
                std::snprintf(idCur,  sizeof(idCur),  "Ch%02d",        row + 1);
            }

            ImPlot::SetNextLineStyle(lineColor, 1.0f);
            if (!buf.xsPrev.empty())
                ImPlot::PlotLine(idPrev, buf.xsPrev.data(), buf.ysPrev.data(),
                                 (int)buf.xsPrev.size());

            ImPlot::SetNextLineStyle(lineColor, 1.0f);
            if (!buf.xsCur.empty())
                ImPlot::PlotLine(idCur, buf.xsCur.data(), buf.ysCur.data(),
                                 (int)buf.xsCur.size());
        }

        // Wiper (OK to use plot queries; NO Setup* calls below)
        ImDrawList* dl = ImPlot::GetPlotDrawList();
        ImVec2 ppos = ImPlot::GetPlotPos();
        ImVec2 psz  = ImPlot::GetPlotSize();
        ImVec2 pc   = ImPlot::PlotToPixels(ImPlotPoint(cursorX, 0.0));
        int cx      = (int)std::floor(pc.x);
        int x0 = (int)std::max(ppos.x, (float)cx);
        int x1 = (int)std::min(ppos.x + psz.x, (float)(cx + kWiperWidthPx));
        if (x1 > x0) {
            ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
            if (bg.w == 0.0f) bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            bg.w = 1.0f;
            dl->AddRectFilled(ImVec2((float)x0, ppos.y),
                              ImVec2((float)x1, ppos.y + psz.y),
                              ImGui::GetColorU32(bg));
        }

        // Left labels
        const ImPlotRect lim = ImPlot::GetPlotLimits();
        const double xLeft = lim.X.Min;
        for (int row = 0; row < rows; ++row) {
            const elda::models::Channel* meta =
                (useSelected && row < (int)selectedChannels.size()) ? selectedChannels[row] : nullptr;

            char label[64];
            if (meta && !meta->name.empty())
                std::snprintf(label, sizeof(label), "%s", meta->name.c_str());
            else
                std::snprintf(label, sizeof(label), "ch%02d", row + 1);

            const double yBase = kTopPadPx + (row + 0.5) * std::min<double>(
                kPreferredRowPx,
                std::max<double>(kMinRowPx, (availH_gui - kTopPadPx) / std::max(1, rows))
            );

            ImPlot::PushStyleColor(ImPlotCol_InlayText, kLabelColorNormal);
            ImPlot::PlotText(label, xLeft, yBase, ImVec2(+kLeftLabelInsetPx, 0.0f), ImPlotTextFlags_None);
            ImPlot::PopStyleColor();
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar(5);
    ImGui::EndChild();
}
