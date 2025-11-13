#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <map>
#include "core/app_state_manager.h"
#include "models/channel.h"

namespace elda::impedance_viewer {

    // Normalized [0..1] position on the canvas
    struct ElectrodePosition {
        float x = 0.5f;
        float y = 0.5f;
        std::string channelId;   // which channel is assigned
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
        void InitializeFromChannels(); // ✅ NEW: Load positions from real channels
        void UpdateElectrodePosition(size_t index, float x, float y); // normalized coords (temporary)
        void StartDragging(size_t index);
        void StopDragging(size_t index);
        void SelectElectrode(int index);
        void ClearSelection();

        // ✅ NEW: Save/Discard operations
        void SavePositionsToState();  // Persist temporary changes to Channel models
        void DiscardChanges();        // Restore original positions from channels

        // Helpers
        const elda::models::Channel* GetChannelById(const std::string& id) const;
        bool IsPositionValid(float x, float y) const; // stay within cap circle

    private:
        void NotifyPositionChanged();
        void InitializeDefaultPositions(); // Fallback for channels without positions

        std::vector<ElectrodePosition> electrodePositions_;      // Current working positions
        std::map<std::string, std::pair<float, float>> originalPositions_; // ✅ NEW: Backup for discard
        std::vector<elda::models::Channel> availableChannels_;   // copied once for stability
        AppStateManager& stateManager_;

        int selectedElectrodeIndex_ = -1;

        // Logical cap radius in normalized space (center 0.5,0.5)
        // We keep this internal constant to validate IsPositionValid()
        const float capRadius_ = 0.48f;
    };

} // namespace elda::impedance_viewer