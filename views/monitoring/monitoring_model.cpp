#include "monitoring_model.h"
#include <cmath>
#include <iostream>

namespace elda {

MonitoringModel::MonitoringModel() {
    initializeBuffers();
}

void MonitoringModel::initializeBuffers() {
    // Initialize ChartData structure directly
    chartData_.numChannels = state_.numChannels;
    chartData_.sampleRateHz = (int)state_.sampleRateHz;
    chartData_.bufferSize = kBufferSize;
    chartData_.amplitudePPuV = state_.amplitudePPuV;
    chartData_.windowSeconds = state_.windowSeconds;
    chartData_.gainMultiplier = state_.gainMultiplier;
    chartData_.playheadSeconds = 0.0;

    // Allocate ring buffer once
    chartData_.ring.tAbs.resize(kBufferSize);
    chartData_.ring.data.resize(state_.numChannels);
    for (auto& ch : chartData_.ring.data) {
        ch.resize(kBufferSize);
    }
    chartData_.ring.write = 0;
    chartData_.ring.filled = false;
}

void MonitoringModel::startAcquisition() {
    std::cout << "[Model] Start acquisition" << std::endl;
    state_.isRunning = true;
    state_.isPaused = false;
    currentTime_ = 0.0;
    chartData_.ring.write = 0;
    chartData_.ring.filled = false;
}

void MonitoringModel::stopAcquisition() {
    std::cout << "[Model] Stop acquisition" << std::endl;
    state_.isRunning = false;
    state_.isPaused = false;
}

void MonitoringModel::pauseAcquisition() {
    std::cout << "[Model] Pause" << std::endl;
    state_.isPaused = true;
}

void MonitoringModel::resumeAcquisition() {
    std::cout << "[Model] Resume" << std::endl;
    state_.isPaused = false;
}

void MonitoringModel::update(float deltaTime) {
    if (state_.isRunning && !state_.isPaused) {
        generateSyntheticData(deltaTime);
        currentTime_ += deltaTime;
        updateChartData();
    }
}

void MonitoringModel::generateSyntheticData(float deltaTime) {
    int samplesThisFrame = (int)(state_.sampleRateHz * deltaTime);

    for (int sample = 0; sample < samplesThisFrame; ++sample) {
        double t = currentTime_ + (sample / state_.sampleRateHz);

        int idx = chartData_.ring.write;
        chartData_.ring.tAbs[idx] = t;

        for (int ch = 0; ch < state_.numChannels; ++ch) {
            double value = 50.0 * std::sin(2.0 * M_PI * 10.0 * t + ch * 0.1);
            value += 20.0 * std::sin(2.0 * M_PI * 20.0 * t + ch * 0.2);
            value += 10.0 * std::sin(2.0 * M_PI * 2.0 * t);
            value += ((rand() % 100 - 50) / 10.0);

            chartData_.ring.data[ch][idx] = value;
        }

        chartData_.ring.write = (chartData_.ring.write + 1) % kBufferSize;
        if (chartData_.ring.write == 0) {
            chartData_.ring.filled = true;
        }
    }
}

void MonitoringModel::updateChartData() {
    // Update display settings (might change via UI)
    chartData_.amplitudePPuV = state_.amplitudePPuV;
    chartData_.windowSeconds = state_.windowSeconds;
    chartData_.gainMultiplier = state_.gainMultiplier;
    chartData_.playheadSeconds = currentTime_;
}

void MonitoringModel::setWindowSeconds(double seconds) {
    state_.windowSeconds = seconds;
}

void MonitoringModel::setAmplitude(double ppuV) {
    state_.amplitudePPuV = ppuV;
}

void MonitoringModel::setGain(double gain) {
    state_.gainMultiplier = gain;
}

} // namespace elda