#include "impedance_viewer_view.h"
#include "impedance_viewer_toolbar.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cmath>

#include "views/impedance_viewer/impedance_viewer_helper.h"
#include "UI/impedance_range/impedance_range.h"

namespace elda::views::impedance_viewer {

ImpedanceViewerView::ImpedanceViewerView() {}

void ImpedanceViewerView::Render(const ImpedanceViewerViewData& data,
                                  const ImpedanceViewerViewCallbacks& callbacks)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("ImpedanceViewer", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImpedanceViewerToolbar(callbacks);

    RenderBody_(data, callbacks);

    ImGui::End();
    ImGui::PopStyleVar();
}

void ImpedanceViewerView::RenderBody_(const ImpedanceViewerViewData& data,
                                      const ImpedanceViewerViewCallbacks& callbacks)
{
    const float rangePanelHeight = 110.0f;
    const float availableCapHeight = ImGui::GetContentRegionAvail().y - rangePanelHeight;

    canvasPos_  = ImGui::GetCursorScreenPos();
    canvasSize_ = ImGui::GetContentRegionAvail();

    centerPos_  = ImVec2(
        canvasPos_.x + canvasSize_.x * 0.5f,
        canvasPos_.y + availableCapHeight * 0.5f
    );

    pixelCapRadius_ = std::min(canvasSize_.x, availableCapHeight) * kCapRadiusNormalized_;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton("impedance_canvas",
        canvasSize_,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool canvasHovered = ImGui::IsItemHovered();

    DrawCapOutline(dl, centerPos_, pixelCapRadius_);
    if (kShowGridDefault_) DrawCapGrid(dl, centerPos_, pixelCapRadius_);

    RenderElectrodes(dl, *data.electrodes, *data.availableChannels, data.selectedElectrodeIndex, callbacks);

    if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mp = ImGui::GetMousePos();
        bool hit = false;
        for (const auto& e : *data.electrodes) {
            ImVec2 p = CapNormalizedToScreen(centerPos_, pixelCapRadius_, e.x, e.y);
            if (PointInCircle(mp, p, kElectrodeRadiusPx_)) { hit = true; break; }
        }
        if (!hit && callbacks.onElectrodeMouseDown) callbacks.onElectrodeMouseDown(-1);
    }

    {
        const float panelPad   = 40.0f;
        const float panelWidth = std::min(420.0f, canvasSize_.x * 0.60f);
        const float panelHeight= 86.0f;

        ImVec2 panelPos(centerPos_.x - panelWidth * 0.5f,
                        centerPos_.y + pixelCapRadius_ + panelPad);

        ImGui::SetCursorScreenPos(panelPos);
        ImGui::BeginChild("impedance_range_panel",
                          ImVec2(panelWidth, panelHeight),
                          false,
                          ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse |
                          ImGuiWindowFlags_NoBackground);

        elda::ui::ImpedanceRanges ranges{ 10000.f, 30000.f, 54000.f };
        elda::ui::ImpedanceRangeConfig barCfg;
        barCfg.show_threshold_labels = true;

        elda::ui::DualCursorConfig curCfg;
        curCfg.cursor_color  = IM_COL32(0,0,0,255);
        curCfg.min_gap_ohms  = 500.0f;
        curCfg.draggable     = true;

        static float low  = 10000.f;
        static float high = 30000.f;

        elda::ui::DrawImpedanceRangeDual("imp-range-dual",
                                         low, high,
                                         ranges, barCfg, curCfg,
                                         &low, &high);

        ImGui::EndChild();
    }
}

void ImpedanceViewerView::RenderElectrodes(
    ImDrawList* dl,
    const std::vector<ElectrodePosition>& electrodes,
    const std::vector<elda::models::Channel>& availableChannels,
    int selectedElectrodeIndex,
    const ImpedanceViewerViewCallbacks& callbacks)
{
    for (size_t i = 0; i < electrodes.size(); ++i) {
        const auto& e = electrodes[i];

        const elda::models::Channel* ch = nullptr;
        if (!e.channelId.empty()) {
            auto it = std::find_if(availableChannels.begin(), availableChannels.end(),
                [&](const elda::models::Channel& c){ return c.id == e.channelId; });
            if (it != availableChannels.end()) ch = &(*it);
        }

        bool isSel = (static_cast<int>(i) == selectedElectrodeIndex);
        RenderSingleElectrode(dl, i, e, ch, isSel, callbacks);
    }
}

void ImpedanceViewerView::RenderSingleElectrode(
    ImDrawList* dl,
    size_t index,
    const ElectrodePosition& electrode,
    const elda::models::Channel* channel,
    bool isSelected,
    const ImpedanceViewerViewCallbacks& callbacks)
{
    ImVec2 pos = CapNormalizedToScreen(centerPos_, pixelCapRadius_, electrode.x, electrode.y);
    ImVec2 mp  = ImGui::GetMousePos();

    bool hovered = PointInCircle(mp, pos, kElectrodeRadiusPx_);

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !electrode.isDragging) {
        if (callbacks.onElectrodeMouseDown) callbacks.onElectrodeMouseDown(static_cast<int>(index));
    }

    if (electrode.isDragging) {
        ImVec2 dropNorm = ScreenToCapNormalizedClamped(centerPos_, pixelCapRadius_, mp);
        pos = CapNormalizedToScreen(centerPos_, pixelCapRadius_, dropNorm.x, dropNorm.y);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (callbacks.onElectrodeDropped) callbacks.onElectrodeDropped(index, dropNorm);
        }
    }

    ImU32 base = ChannelColorFromId(channel ? channel->id : std::string());
    if (hovered)  base = IM_COL32(
        std::min(255, (int)( base        & 0xFF) + 15),
        std::min(255, (int)((base >> 8 ) & 0xFF) + 15),
        std::min(255, (int)((base >> 16) & 0xFF) + 15),
        255
    );
    if (isSelected) base = IM_COL32(255,235,150,255);

    float r = kElectrodeRadiusPx_;
    if (isSelected) r *= 1.20f;
    if (hovered)    r *= 1.10f;

    dl->AddCircleFilled(pos, r, base);
    dl->AddCircle(pos, r, IM_COL32(0,0,0,255), 22, 1.0f);

    const char* label = nullptr; char fb[8];
    if (channel) label = channel->name.c_str();
    else { std::snprintf(fb, sizeof(fb), "%zu", index + 1); label = fb; }
    ImVec2 sz = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(pos.x - sz.x * 0.5f, pos.y - sz.y * 0.5f), IM_COL32(0,0,0,255), label);
}

} // namespace elda::impedance_viewer