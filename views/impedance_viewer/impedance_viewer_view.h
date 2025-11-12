#pragma once
#include "imgui.h"
#include "impedance_viewer_model.h"
#include <functional>
#include <vector>

namespace elda::impedance_viewer {

struct ImpedanceViewerViewCallbacks {
    std::function<void(int electrodeIndex)> onElectrodeMouseDown; // -1 = deselect
    std::function<void(size_t electrodeIndex, ImVec2 normalizedDropPos)> onElectrodeDropped;
    std::function<void()> onSave;
    std::function<void()> onClose;
};

class ImpedanceViewerView {
public:
    ImpedanceViewerView();

    // Fullscreen/borderless host (kept for flexibility)
    void Render(bool *isOpen,
                const std::vector<ElectrodePosition> &electrodes,
                const std::vector<elda::models::Channel> &availableChannels,
                int selectedElectrodeIndex, const ImpedanceViewerViewCallbacks &callbacks);

    // NEW: true modal (borderless) with our custom header inside
    void RenderModal(bool* isOpen,
                     const std::vector<ElectrodePosition>& electrodes,
                     const std::vector<elda::models::Channel>& availableChannels,
                     int selectedElectrodeIndex,
                     const ImpedanceViewerViewCallbacks& callbacks);

private:
    // constants
    const float kElectrodeRadiusPx_   = 15.0f;
    const bool  kShowGridDefault_     = true;
    const float kCapRadiusNormalized_ = 0.40f;

    // cached during a frame
    ImVec2 canvasPos_{};
    ImVec2 canvasSize_{};
    ImVec2 centerPos_{};
    float  pixelCapRadius_ = 0.0f;

    // shared body used by both Render & RenderModal
    void RenderBody_(const std::vector<ElectrodePosition>& electrodes,
                     const std::vector<elda::models::Channel>& availableChannels,
                     int selectedElectrodeIndex,
                     const ImpedanceViewerViewCallbacks& callbacks);

    void RenderElectrodes(ImDrawList* drawList,
                          const std::vector<ElectrodePosition>& electrodes,
                          const std::vector<elda::models::Channel>& availableChannels,
                          int selectedElectrodeIndex,
                          const ImpedanceViewerViewCallbacks& callbacks);

    void RenderSingleElectrode(ImDrawList* drawList,
                               size_t index,
                               const ElectrodePosition& electrode,
                               const elda::models::Channel* channel,
                               bool isSelected,
                               const ImpedanceViewerViewCallbacks& callbacks);
};

} // namespace elda::impedance_viewer
