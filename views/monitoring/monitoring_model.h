#ifndef ELDA_MONITORING_MODEL_H
#define ELDA_MONITORING_MODEL_H

#include "UI/chart/chart_data.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include "models/channels_group.h"

namespace elda {

    class MonitoringModel {
    public:
        MonitoringModel(AppState& state, elda::AppStateManager& stateManager);
        ~MonitoringModel() = default;

        // Lifecycle
        void startAcquisition();
        void stopAcquisition();
        void update(float deltaTime);

        // Actions (called by Presenter)
        void toggleMonitoring();
        void toggleRecording();
        void increaseWindow();
        void decreaseWindow();
        void increaseAmplitude();
        void decreaseAmplitude();
        void applyChannelConfiguration(const elda::models::ChannelsGroup& group);

        // Getters (Presenter collects this data for View)
        const ChartData& getChartData() const { return chartData_; }
        bool isMonitoring() const { return stateManager_.IsMonitoring(); }
        bool canRecord() const { return isMonitoring(); }
        bool isRecordingActive() const {
            return stateManager_.IsRecording() && !stateManager_.IsPaused();
        }
        bool isCurrentlyPaused() const {
            return stateManager_.IsRecording() && stateManager_.IsPaused();
        }
        int getWindowSeconds() const {
            return (int)stateManager_.GetWindowSeconds();
        }
        int getAmplitudeMicroVolts() const {
            return stateManager_.GetAmplitudeMicroVolts();
        }
        double getSampleRateHz() const { return SAMPLE_RATE_HZ; }

        const std::vector<elda::models::ChannelsGroup>& getAvailableGroups() const {
            return state_.availableGroups;
        }

        int getActiveGroupIndex() const;

    private:
        AppState& state_;
        elda::AppStateManager& stateManager_;
        ChartData chartData_;
        static constexpr int kBufferSize = 25000;

        void initializeBuffers();
        void generateSyntheticData(float deltaTime);
        void updateChartData();
    };

} // namespace elda

#endif // ELDA_MONITORING_MODEL_H