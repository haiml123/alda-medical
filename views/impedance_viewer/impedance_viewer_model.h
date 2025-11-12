#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include "core/app_state_manager.h"
#include "models/channel.h"

namespace elda::impedance_viewer {

    // Normalized [0..1] position on the canvas
    struct ElectrodePosition {
        float x = 0.5f;
        float y = 0.5f;
        std::string channelId;   // optional: which channel is assigned
        bool isDragging = false; // controlled by Presenter
    };

    class ImpedanceViewerModel {
    public:
        ImpedanceViewerModel(
            const std::vector<elda::models::Channel>& availableChannels,
            AppStateManager& stateManager);

        // State access
        const std::vector<ElectrodePosition>& GetElectrodePositions() const { return electrodePositions_; }
        const std::vector<elda::models::Channel>& GetAvailableChannels() const { return availableChannels_; }
        int  GetSelectedElectrodeIndex() const { return selectedElectrodeIndex_; }

        // Ops
        void InitializeDefaultPositions(); // 20 nodes in concentric circles
        void UpdateElectrodePosition(size_t index, float x, float y); // normalized coords
        void StartDragging(size_t index);
        void StopDragging(size_t index);
        void SelectElectrode(int index);
        void ClearSelection();

        // Helpers
        const elda::models::Channel* GetChannelById(const std::string& id) const;
        bool IsPositionValid(float x, float y) const; // stay within cap circle

    private:
        void NotifyPositionChanged();

        std::vector<ElectrodePosition> electrodePositions_;
        std::vector<elda::models::Channel> availableChannels_; // copied once for stability
        AppStateManager& stateManager_;

        int selectedElectrodeIndex_ = -1;

        // Logical cap radius in normalized space (center 0.5,0.5)
        // We keep this internal constant to validate IsPositionValid()
        const float capRadius_ = 0.40f;
    };

} // namespace elda::impedance_viewer
