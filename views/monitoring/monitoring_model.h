#ifndef ELDA_MONITORING_MODEL_H
#define ELDA_MONITORING_MODEL_H

#include "UI/chart/chart_data.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include "models/channels_group.h"
#include "models/mvp_base_model.h"
#include "services/channel_management_service.h"

namespace elda::views::monitoring {

    class MonitoringModel : public models::MVPBaseModel {
    public:
        MonitoringModel(AppState& state, elda::AppStateManager& stateManager);
        ~MonitoringModel() = default;

        // Lifecycle
        void StartAcquisition();
        void StopAcquisition();
        void Update(float deltaTime);

        // Actions (called by Presenter)
        void ToggleMonitoring() const;

        void StopRecording() const;

        void ToggleRecording() const;
        void IncreaseWindow() const;
        void DecreaseWindow() const;
        void IncreaseAmplitude() const;
        void DecreaseAmplitude() const;
        void ApplyChannelConfiguration(const elda::models::ChannelsGroup& group) const;

        // ✅ NEW: Refresh available groups from service
        void RefreshAvailableGroups() const;

        void OnGroupSelected(const models::ChannelsGroup &group) const;

        std::vector<const models::Channel *>& GetSelectedChannels() const;

        // Getters (Presenter collects this data for View)
        const ChartData& GetChartData() const { return chartData_; }
        bool IsMonitoring() const { return stateManager_.IsMonitoring(); }
        bool CanRecord() const { return IsMonitoring(); }
        bool IsRecordingActive() const {
            return stateManager_.IsRecordingActive() && stateManager_.IsMonitoring();
        }

        RecordingState GetRecordingState() const { return stateManager_.GetRecordingState(); }
        bool IsCurrentlyPaused() const {
            return stateManager_.IsPaused() && stateManager_.IsMonitoring();
        }
        bool IsCurrentlyRecording() const {
            return stateManager_.IsRecording() && stateManager_.IsMonitoring();
        }
        bool IsStopped() const { return stateManager_.IsStopped(); }
        int GetWindowSeconds() const {
            return (int)stateManager_.GetWindowSeconds();
        }
        int GetAmplitudeMicroVolts() const {
            return stateManager_.GetAmplitudeMicroVolts();
        }
        double GetSampleRateHz() const { return SAMPLE_RATE_HZ; }

        const std::vector<elda::models::ChannelsGroup>& GetAvailableGroups() const {
            return state_.availableGroups;
        }

        int GetActiveGroupIndex() const;

    private:
        AppState& state_;
        elda::AppStateManager& stateManager_;
        ChartData chartData_;
        static constexpr int kBufferSize = 25000;

        void InitializeBuffers();
        void GenerateSyntheticData(float deltaTime);
        void UpdateChartData();
    };

} // namespace elda

#endif // ELDA_MONITORING_MODEL_H
