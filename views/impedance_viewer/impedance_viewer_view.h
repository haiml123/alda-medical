#pragma once
#include "imgui.h"
#include "impedance_viewer_model.h"
#include <functional>
#include <vector>

namespace elda::views::impedance_viewer {

    struct ImpedanceViewerViewCallbacks {
        std::function<void(int electrodeIndex)> onElectrodeMouseDown;
        std::function<void(size_t electrodeIndex, ImVec2 normalizedDropPos)> onElectrodeDropped;
        std::function<void()> onSave;
        std::function<void()> onRedirectToSettings;
        std::function<void()> onRedirectToMonitoring;
    };

    struct ImpedanceViewerViewData {
        const std::vector<ElectrodePosition>* electrodes = nullptr;
        const std::vector<elda::models::Channel>* availableChannels = nullptr;
        int selectedElectrodeIndex = -1;
    };

    class ImpedanceViewerView {
    public:
        ImpedanceViewerView();

        void Render(const ImpedanceViewerViewData& data,
                    const ImpedanceViewerViewCallbacks& callbacks);

    private:
        const float kElectrodeRadiusPx_   = 15.0f;
        const bool  kShowGridDefault_     = true;
        const float kCapRadiusNormalized_ = 0.40f;

        ImVec2 canvasPos_{};
        ImVec2 canvasSize_{};
        ImVec2 centerPos_{};
        float  pixelCapRadius_ = 0.0f;

        void RenderBody_(const ImpedanceViewerViewData& data,
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