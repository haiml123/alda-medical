#include "impedance_viewer_view.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cmath>

#include "views/impedance_viewer/impedance_viewer_header.h"
#include "views/impedance_viewer/impedance_viewer_helper.h"
#include "UI/impedance_range/impedance_range.h"

namespace elda::impedance_viewer {

ImpedanceViewerView::ImpedanceViewerView() {}


// ===== Fullscreen/borderless host (optional) =====
// Fullscreen/borderless host (window, not popup)
void ImpedanceViewerView::Render(bool* isOpen,
                                 const std::vector<ElectrodePosition>& electrodes,
                                 const std::vector<elda::models::Channel>& availableChannels,
                                 int selectedElectrodeIndex,
                                 const ImpedanceViewerViewCallbacks& callbacks)
{
    if (!isOpen) return;  // don't draw if closed

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 size(viewport->WorkSize.x * 0.40f, viewport->WorkSize.y * 0.90f);
    ImVec2 pos (viewport->WorkPos.x + (viewport->WorkSize.x - size.x) * 0.5f,
                viewport->WorkPos.y + (viewport->WorkSize.y - size.y) * 0.5f);

    ImGui::SetNextWindowPos(pos,  ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

    // IMPORTANT: pass p_open (&open) so we can close by flipping it
    bool open = *isOpen;
    ImGui::Begin("ImpedanceViewer", &open,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::BeginChild("impedance_viewer_root",
                      ImVec2(0, 0),
                      ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Header with Close/Save callbacks
    elda::impedance_viewer::ui::HeaderCallbacks hcb;
    hcb.onSave  = callbacks.onSave;
    hcb.onClose = [&, isOpen]() {
        if (callbacks.onClose) callbacks.onClose();
        *isOpen = false;            // << this is what "closes" the window
        // Note: CloseCurrentPopup() is for popups only, not normal windows.
    };

    elda::impedance_viewer::ui::RenderImpedanceViewerHeader("Impedance Viewer", hcb, 30.0f);
    RenderBody_(electrodes, availableChannels, selectedElectrodeIndex, callbacks);

    ImGui::EndChild();
    ImGui::End();
}


// ===== Shared body =====
void ImpedanceViewerView::RenderBody_(const std::vector<ElectrodePosition>& electrodes,
                                      const std::vector<elda::models::Channel>& availableChannels,
                                      int selectedElectrodeIndex,
                                      const ImpedanceViewerViewCallbacks& callbacks)
{
    // Layout
    canvasPos_  = ImGui::GetCursorScreenPos();
    canvasSize_ = ImGui::GetContentRegionAvail();
    centerPos_  = ImVec2(canvasPos_.x + canvasSize_.x * 0.5f,
                         canvasPos_.y + canvasSize_.y * 0.5f);
    pixelCapRadius_ = std::min(canvasSize_.x, canvasSize_.y) * kCapRadiusNormalized_;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Canvas hit target
    ImGui::InvisibleButton("impedance_canvas",
        canvasSize_,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool canvasHovered = ImGui::IsItemHovered();

    // Head
    helper::DrawCapOutline(dl, centerPos_, pixelCapRadius_);
    if (kShowGridDefault_) helper::DrawCapGrid(dl, centerPos_, pixelCapRadius_);

    // Electrodes
    RenderElectrodes(dl, electrodes, availableChannels, selectedElectrodeIndex, callbacks);

    // Deselect on empty click
    if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mp = ImGui::GetMousePos();
        bool hit = false;
        for (const auto& e : electrodes) {
            ImVec2 p = helper::CapNormalizedToScreen(centerPos_, pixelCapRadius_, e.x, e.y);
            if (helper::PointInCircle(mp, p, kElectrodeRadiusPx_)) { hit = true; break; }
        }
        if (!hit && callbacks.onElectrodeMouseDown) callbacks.onElectrodeMouseDown(-1);
    }

    // Range widget (below head)
    {
        const float panelPad   = 12.0f;
        const float panelWidth = std::min(420.0f, canvasSize_.x * 0.60f);
        const float panelHeight= 86.0f;

        ImVec2 panelPos(centerPos_.x - panelWidth * 0.5f,
                        centerPos_.y + pixelCapRadius_ + panelPad);

        ImGui::SetCursorScreenPos(panelPos);
        ImGui::BeginChild("impedance_range_panel",
                          ImVec2(panelWidth, panelHeight),
                          true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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
    ImVec2 pos = helper::CapNormalizedToScreen(centerPos_, pixelCapRadius_, electrode.x, electrode.y);
    ImVec2 mp  = ImGui::GetMousePos();

    bool hovered = helper::PointInCircle(mp, pos, kElectrodeRadiusPx_);

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !electrode.isDragging) {
        if (callbacks.onElectrodeMouseDown) callbacks.onElectrodeMouseDown(static_cast<int>(index));
    }

    if (electrode.isDragging) {
        ImVec2 dropNorm = helper::ScreenToCapNormalizedClamped(centerPos_, pixelCapRadius_, mp);
        pos = helper::CapNormalizedToScreen(centerPos_, pixelCapRadius_, dropNorm.x, dropNorm.y);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (callbacks.onElectrodeDropped) callbacks.onElectrodeDropped(index, dropNorm);
        }
    }

    ImU32 base = helper::ChannelColorFromId(channel ? channel->id : std::string());
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
