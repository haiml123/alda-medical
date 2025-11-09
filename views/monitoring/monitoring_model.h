#ifndef ELDA_MONITORING_MODEL_H
#define ELDA_MONITORING_MODEL_H

#include "UI/chart/chart_data.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include "models/channels_group.h"

namespace elda {

    /**
     * ViewModel for toolbar - exposes only what View needs
     * No direct AppState access for View
     */
    struct ToolbarViewModel {
        // Button states
        bool monitoring = false;
        bool canRecord = false;
        bool recordingActive = false;
        bool currentlyPaused = false;

        // Display values
        int windowSeconds = 10;
        int amplitudeMicroVolts = 100;
        int winIdx = 0;
        int ampIdx = 0;

        // System info
        double sampleRateHz = 1000.0;
        float fps = 60.0f;
    };

    class MonitoringModel {
    public:
        MonitoringModel(AppState& state, elda::AppStateManager& stateManager);
        ~MonitoringModel() = default;

        // === ACQUISITION CONTROL ===
        void startAcquisition();
        void stopAcquisition();
        void pauseAcquisition();
        void resumeAcquisition();
        void update(float deltaTime);

        // === TOOLBAR BUSINESS LOGIC ===

        // Monitor control
        void toggleMonitoring();

        // Recording control
        void toggleRecording();

        // Display controls
        void increaseWindow();
        void decreaseWindow();
        void increaseAmplitude();
        void decreaseAmplitude();

        // Channel configuration - accepts ChannelsGroup
        void applyChannelConfiguration(const elda::models::ChannelsGroup& group);

        // === DATA ACCESS (Read-only for View) ===

        const ChartData& getChartData() const { return chartData_; }
        ToolbarViewModel getToolbarViewModel() const;

    private:
        // References to core state (Model layer only!)
        AppState& state_;
        elda::AppStateManager& stateManager_;

        // Internal state
        ChartData chartData_;
        static constexpr int kBufferSize = 25000;

        void initializeBuffers();
        void generateSyntheticData(float deltaTime);
        void updateChartData();
    };

} // namespace elda

#endif