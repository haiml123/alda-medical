#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <map>
#include "core/app_state_manager.h"
#include "models/channel.h"

namespace elda::views::impedance_viewer {

    struct ElectrodePosition {
        float x = 0.5f;
        float y = 0.5f;
        std::string channelId;
        bool isDragging = false;
    };

    class ImpedanceViewerModel {
    public:
        ImpedanceViewerModel(
            const std::vector<elda::models::Channel>& availableChannels,
            AppStateManager& stateManager);

        const std::vector<ElectrodePosition>& GetElectrodePositions() const { return electrodePositions_; }
        const std::vector<elda::models::Channel>& GetAvailableChannels() const { return availableChannels_; }
        int  GetSelectedElectrodeIndex() const { return selectedElectrodeIndex_; }

        void Update();
        void InitializeFromChannels();
        void UpdateElectrodePosition(size_t index, float x, float y);
        void StartDragging(size_t index);
        void StopDragging(size_t index);
        void SelectElectrode(int index);
        void ClearSelection();

        void SavePositionsToState();
        void DiscardChanges();

        const elda::models::Channel* GetChannelById(const std::string& id) const;
        bool IsPositionValid(float x, float y) const;

    private:
        void NotifyPositionChanged();
        void InitializeDefaultPositions();

        std::vector<ElectrodePosition> electrodePositions_;
        std::map<std::string, std::pair<float, float>> originalPositions_;
        std::vector<elda::models::Channel> availableChannels_;
        AppStateManager& stateManager_;

        int selectedElectrodeIndex_ = -1;
        const float capRadius_ = 0.48f;
    };

} // namespace elda::impedance_viewer