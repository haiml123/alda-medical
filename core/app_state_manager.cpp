#include "app_state_manager.h"

namespace elda {

// ===== CONSTRUCTOR / DESTRUCTOR =====

AppStateManager::AppStateManager(AppState& state)
    : state_(state)
    , impedanceCheckPassed_(false) {
}

AppStateManager::~AppStateManager() {
}

// ===== MONITORING CONTROL =====

StateChangeError AppStateManager::SetMonitoring(bool enable) {
    if (state_.isMonitoring == enable) {
        return {StateChangeResult::Success, ""};
    }

    if (enable) {
        std::string errorMsg;
        if (!ValidateCanStartMonitoring(errorMsg)) {
            return {StateChangeResult::ValidationFailed, errorMsg};
        }

        state_.isMonitoring = true;
        NotifyStateChanged(StateField::Monitoring);
    } else {
        if (state_.isRecordingToFile) {
            auto result = StopRecording();
            // Continue stopping monitoring even if recording stop fails
        }

        state_.isMonitoring = false;
        NotifyStateChanged(StateField::Monitoring);
    }

    return {StateChangeResult::Success, ""};
}

// ===== RECORDING CONTROL =====

StateChangeError AppStateManager::StartRecording() {
    if (state_.isRecordingToFile) {
        return {StateChangeResult::InvalidTransition, "Already recording"};
    }

    std::string errorMsg;
    if (!ValidateCanStartRecording(errorMsg)) {
        if (!impedanceCheckPassed_) {
            return {StateChangeResult::ImpedanceCheckRequired, errorMsg};
        }
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    state_.isRecordingToFile = true;
    state_.isPaused = false;
    state_.recordingStartTime = state_.currentEEGTime();

    NotifyStateChanged(StateField::Recording);

    // TODO: Initialize file writer here when hardware integration is complete

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::StopRecording() {
    if (!state_.isRecordingToFile) {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    state_.isRecordingToFile = false;
    state_.isPaused = false;

    NotifyStateChanged(StateField::Recording);

    // TODO: Finalize and close file writer

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::PauseRecording() {
    if (!state_.isRecordingToFile) {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    if (state_.isPaused) {
        return {StateChangeResult::InvalidTransition, "Already paused"};
    }

    state_.isPaused = true;
    state_.pauseMarks.push_back({state_.currentEEGTime()});

    NotifyStateChanged(StateField::Paused);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::ResumeRecording() {
    if (!state_.isRecordingToFile) {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    if (!state_.isPaused) {
        return {StateChangeResult::InvalidTransition, "Not currently paused"};
    }

    state_.isPaused = false;

    NotifyStateChanged(StateField::Paused);

    return {StateChangeResult::Success, ""};
}

// ===== CHANNEL CONFIGURATION =====

StateChangeError AppStateManager::SetChannelConfiguration(
    const std::string& groupName,
    const std::vector<models::Channel>& channels
) {
    std::string errorMsg;
    if (!ValidateCanChangeChannels(errorMsg)) {
        return {StateChangeResult::InvalidTransition, errorMsg};
    }

    if (channels.empty()) {
        return {StateChangeResult::ValidationFailed,
                "Must select at least one channel"};
    }

    if (channels.size() > 136) {
        return {StateChangeResult::ValidationFailed,
                "Maximum 136 channels supported"};
    }

    state_.currentChannelGroupName = groupName;
    state_.selectedChannels = channels;

    // Impedance check is invalidated when channels change
    impedanceCheckPassed_ = false;

    NotifyStateChanged(StateField::ChannelConfig);

    return {StateChangeResult::Success, ""};
}

// ===== DISPLAY SETTINGS =====

StateChangeError AppStateManager::SetDisplayWindow(int windowIndex) {
    std::string errorMsg;
    if (!ValidateWindowIndex(windowIndex, errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    int oldIdx = state_.winIdx;
    state_.winIdx = windowIndex;
    state_.lastWinIdx = oldIdx;

    NotifyStateChanged(StateField::DisplayWindow);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::SetDisplayAmplitude(int amplitudeIndex) {
    std::string errorMsg;
    if (!ValidateAmplitudeIndex(amplitudeIndex, errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    state_.ampIdx = amplitudeIndex;

    NotifyStateChanged(StateField::DisplayAmplitude);

    return {StateChangeResult::Success, ""};
}

// ===== NOISE/ARTIFACT CONTROL =====

StateChangeError AppStateManager::SetNoiseScale(float scale) {
    std::string errorMsg;
    if (!ValidateScale(scale, "noise_scale", errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    state_.noiseScale = scale;

    NotifyStateChanged(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::SetArtifactScale(float scale) {
    std::string errorMsg;
    if (!ValidateScale(scale, "artifact_scale", errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    state_.artifactScale = scale;

    NotifyStateChanged(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

// ===== OBSERVER PATTERN =====

AppStateManager::ObserverHandle AppStateManager::AddObserver(StateObserver observer) {
    ObserverHandle handle = nextHandle_++;
    observers_.push_back({handle, observer});
    return handle;
}

void AppStateManager::RemoveObserver(ObserverHandle handle) {
    auto it = std::remove_if(observers_.begin(), observers_.end(),
        [handle](const auto& pair) { return pair.first == handle; });
    observers_.erase(it, observers_.end());
}

void AppStateManager::ClearObservers() {
    observers_.clear();
}

void AppStateManager::NotifyStateChanged(StateField field) {
    for (const auto& [handle, observer] : observers_) {
        observer(field);
    }
}

// ===== VALIDATION HELPERS =====

bool AppStateManager::ValidateCanStartMonitoring(std::string& errorMsg) {
    if (state_.selectedChannels.empty()) {
        errorMsg = "No channels selected - configure channels before monitoring";
        return false;
    }

    // TODO: Add hardware connectivity check when SDK is integrated

    return true;
}

bool AppStateManager::ValidateCanStartRecording(std::string& errorMsg) {
    if (!state_.isMonitoring) {
        errorMsg = "Cannot record while not monitoring";
        return false;
    }

    if (state_.selectedChannels.empty()) {
        errorMsg = "No channels selected";
        return false;
    }

    if (!impedanceCheckPassed_) {
        errorMsg = "Impedance check required - all channels must be <50kΩ";
    }

    return true;
}

bool AppStateManager::ValidateCanChangeChannels(std::string& errorMsg) {
    if (state_.isRecordingToFile && !state_.isPaused) {
        errorMsg = "Cannot change channel configuration during active recording";
        return false;
    }

    return true;
}

bool AppStateManager::ValidateWindowIndex(int index, std::string& errorMsg) {
    if (index < 0 || index >= WINDOW_COUNT) {
        errorMsg = "Invalid window index (must be 0-" + std::to_string(WINDOW_COUNT - 1) + ")";
        return false;
    }
    return true;
}

bool AppStateManager::ValidateAmplitudeIndex(int index, std::string& errorMsg) {
    if (index < 0 || index >= AMP_COUNT) {
        errorMsg = "Invalid amplitude index (must be 0-" + std::to_string(AMP_COUNT - 1) + ")";
        return false;
    }
    return true;
}

bool AppStateManager::ValidateScale(float scale, const std::string& paramName,
                                    std::string& errorMsg) {
    if (scale < 0.0f || scale > 5.0f) {
        errorMsg = paramName + " must be between 0.0 and 5.0";
        return false;
    }
    return true;
}

} // namespace elda