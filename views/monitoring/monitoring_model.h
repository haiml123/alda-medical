#ifndef ELDA_MONITORING_MODEL_H
#define ELDA_MONITORING_MODEL_H

#include "UI/chart/chart_data.h"

namespace elda {

    class MonitoringModel {
    public:
        struct State {
            bool isRunning = false;
            bool isPaused = false;
            double sampleRateHz = 1000.0;
            int numChannels = 64;
            double windowSeconds = 10.0;
            double amplitudePPuV = 100.0;
            double gainMultiplier = 1.0;
        };

        MonitoringModel();
        ~MonitoringModel() = default;

        void startAcquisition();
        void stopAcquisition();
        void pauseAcquisition();
        void resumeAcquisition();
        void update(float deltaTime);

        void setWindowSeconds(double seconds);
        void setAmplitude(double ppuV);
        void setGain(double gain);

        const State& getState() const { return state_; }

        // Return const reference - NO COPYING!
        const ChartData& getChartData() const { return chartData_; }

    private:
        State state_;
        ChartData chartData_;  // Store directly
        double currentTime_ = 0.0;
        static constexpr int kBufferSize = 25000;

        void initializeBuffers();
        void generateSyntheticData(float deltaTime);
        void updateChartData();  // Update chartData_ fields
    };

} // namespace elda

#endif