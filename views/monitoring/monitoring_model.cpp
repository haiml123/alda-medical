#include "monitoring_model.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace elda {

MonitoringModel::MonitoringModel(AppState& state, elda::AppStateManager& stateManager)
    : models::MVPBaseModel(stateManager), state_(state), stateManager_(stateManager) {
    InitializeBuffers();
}

void MonitoringModel::InitializeBuffers() {
    chartData_.numChannels = CHANNELS;
    chartData_.sampleRateHz = (int)SAMPLE_RATE_HZ;
    chartData_.bufferSize = kBufferSize;
    chartData_.amplitudePPuV = state_.ampPPuV();
    chartData_.windowSeconds = state_.windowSec();
    chartData_.gainMultiplier = state_.gainMul();
    chartData_.playheadSeconds = 0.0;

    chartData_.ring.tAbs.clear();
    chartData_.ring.data.clear();
    chartData_.ring.write = 0;
    chartData_.ring.filled = false;
}

void MonitoringModel::StartAcquisition() {
    std::cout << "[Model] Start acquisition" << std::endl;
}

void MonitoringModel::StopAcquisition() {
    std::cout << "[Model] Stop acquisition" << std::endl;
}

void MonitoringModel::Update(float deltaTime) {
    if (stateManager_.IsMonitoring()) {
        state_.tickDisplay(true);
        UpdateChartData();
        GenerateSyntheticData(deltaTime);
    } else {
        state_.tickDisplay(false);
    }
}

void MonitoringModel::GenerateSyntheticData(float /*deltaTime*/) {
    static SynthEEG synthGen;
    std::vector<float> sample(CHANNELS);

    int samplesThisFrame = state_.sampler.due();

    for (int i = 0; i < samplesThisFrame; ++i) {
        synthGen.next(sample);

        for (int ch = 0; ch < CHANNELS; ++ch) {
            sample[ch] *= state_.noiseScale;
        }

        state_.ring.push(sample);
    }
}

void MonitoringModel::UpdateChartData() {
    chartData_.amplitudePPuV = state_.ampPPuV();
    chartData_.windowSeconds = state_.windowSec();
    chartData_.gainMultiplier = state_.gainMul();
    chartData_.playheadSeconds = state_.ring.now;

    chartData_.ring.tAbs.resize(BUFFER_SIZE);
    chartData_.ring.data.resize(CHANNELS);

    for (int i = 0; i < BUFFER_SIZE; ++i) {
        chartData_.ring.tAbs[i] = static_cast<double>(state_.ring.tAbs[i]);
    }

    for (int ch = 0; ch < CHANNELS; ++ch) {
        chartData_.ring.data[ch].resize(BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; ++i) {
            chartData_.ring.data[ch][i] = static_cast<double>(state_.ring.data[ch][i]);
        }
    }

    chartData_.ring.write = state_.ring.write;
    chartData_.ring.filled = state_.ring.filled;
    chartData_.bufferSize = BUFFER_SIZE;
}

// ============================================================================
// ACTIONS
// ============================================================================

void MonitoringModel::ToggleMonitoring() const {
    const bool monitoring = stateManager_.IsMonitoring();
    std::printf("[Model] ToggleMonitoring - current: %s\n",
               monitoring ? "MONITORING" : "IDLE");

    auto result = stateManager_.SetMonitoring(!monitoring);
    if (!result.IsSuccess()) {
        std::fprintf(stderr, "[Model] Monitor toggle failed: %s\n", result.message.c_str());
    } else {
        std::printf("[Model] Monitor toggle SUCCESS - new: %s\n",
                   !monitoring ? "MONITORING" : "IDLE");

        if (!monitoring) {
            std::printf("[Model] Resetting ring buffer\n");
            state_.ring.reset();
            state_.playheadSeconds = 0.0;
            state_.sampler = SampleClock(SAMPLE_RATE_HZ);
        }
    }
}

void MonitoringModel::StopRecording() const {
    stateManager_.StopRecording();
}

void MonitoringModel::ToggleRecording() const {
    const bool recordingActive = stateManager_.IsRecording();
    const bool currentlyPaused = stateManager_.IsPaused();

    if (recordingActive && !currentlyPaused) {
        auto result = stateManager_.PauseRecording();
        if (!result.IsSuccess()) {
            std::fprintf(stderr, "[Model] Pause failed: %s\n", result.message.c_str());
        }
    } else {
        elda::StateChangeError result;
        if (currentlyPaused) {
            result = stateManager_.ResumeRecording();
        } else {
            result = stateManager_.StartRecording();
            if (result.result == elda::StateChangeResult::ImpedanceCheckRequired) {
                std::fprintf(stderr, "[Model] Recording requires impedance check first\n");
            }
        }

        if (!result.IsSuccess()) {
            std::fprintf(stderr, "[Model] Record toggle failed: %s\n", result.message.c_str());
        }
    }
}

void MonitoringModel::IncreaseWindow() const {
    if (state_.winIdx < WINDOW_COUNT - 1) {
        stateManager_.SetDisplayWindow(state_.winIdx + 1);
    }
}

void MonitoringModel::DecreaseWindow() const {
    if (state_.winIdx > 0) {
        stateManager_.SetDisplayWindow(state_.winIdx - 1);
    }
}

void MonitoringModel::IncreaseAmplitude() const {
    if (state_.ampIdx < AMP_COUNT - 1) {
        stateManager_.SetDisplayAmplitude(state_.ampIdx + 1);
    }
}

void MonitoringModel::DecreaseAmplitude() const {
    if (state_.ampIdx > 0) {
        stateManager_.SetDisplayAmplitude(state_.ampIdx - 1);
    }
}

void MonitoringModel::RefreshAvailableGroups() const {
    auto& service = elda::services::ChannelManagementService::GetInstance();
    state_.availableGroups = service.GetAllChannelGroups();

    std::printf("[MonitoringModel] ✓ Available groups refreshed, total: %zu\n",
               state_.availableGroups.size());
}

void MonitoringModel::OnGroupSelected(const models::ChannelsGroup& group) const {
    stateManager_.SetActiveChannelGroup(group);
}

std::vector<const models::Channel *>& MonitoringModel::GetSelectedChannels() const {
    return stateManager_.GetSelectedChannels();
}

void MonitoringModel::ApplyChannelConfiguration(const elda::models::ChannelsGroup& group) const {
    RefreshAvailableGroups();
}

// ============================================================================
// TAB BAR SUPPORT
// ============================================================================

int MonitoringModel::GetActiveGroupIndex() const {
    const std::string& activeName = state_.currentChannelGroupName;
    const auto& groups = state_.availableGroups;

    auto it = std::find_if(groups.begin(), groups.end(),
        [&activeName](const elda::models::ChannelsGroup& g) {
            return g.name == activeName;
        });

    if (it != groups.end()) {
        return std::distance(groups.begin(), it);
    }

    if (!activeName.empty()) {
        std::fprintf(stderr, "[Model] Warning: Active group '%s' not found\n",
                    activeName.c_str());
    }
    return 0;
}

} // namespace elda
