#include "app_state_manager.h"
#include <iomanip>
#include <sstream>
#include <cstdio>

namespace elda {

// ===== CONSTRUCTOR / DESTRUCTOR =====

AppStateManager::AppStateManager(AppState& state)
    : state_(state)
    , impedanceCheckPassed_(false)
    , auditLogEnabled_(false) {

    std::printf("[StateManager] Initialized\n");
}

AppStateManager::~AppStateManager() {
    if (auditLogFile_.is_open()) {
        auditLogFile_.close();
    }
    std::printf("[StateManager] Destroyed\n");
}

// ===== MONITORING CONTROL =====

StateChangeError AppStateManager::SetMonitoring(bool enable) {
    if (state_.isMonitoring == enable) {
        // Already in desired state
        return {StateChangeResult::Success, ""};
    }

    if (enable) {
        // Starting monitoring
        std::string errorMsg;
        if (!ValidateCanStartMonitoring(errorMsg)) {
            return {StateChangeResult::ValidationFailed, errorMsg};
        }

        state_.isMonitoring = true;
        LogStateChange("monitoring", false, true);
        NotifyStateChanged(StateField::Monitoring);

        std::printf("[StateManager] Monitoring STARTED at t=%.3f\n",
                   state_.currentEEGTime());

    } else {
        // Stopping monitoring

        // If recording, stop it first
        if (state_.isRecordingToFile) {
            auto result = StopRecording();
            if (!result.IsSuccess()) {
                std::fprintf(stderr, "[StateManager] Warning: Failed to stop recording: %s\n",
                           result.message.c_str());
            }
        }

        state_.isMonitoring = false;
        LogStateChange("monitoring", true, false);
        NotifyStateChanged(StateField::Monitoring);

        std::printf("[StateManager] Monitoring STOPPED at t=%.3f\n",
                   state_.currentEEGTime());
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

    // Perform state change
    state_.isRecordingToFile = true;
    state_.isPaused = false;
    state_.recordingStartTime = state_.currentEEGTime();

    LogStateChange("recording", false, true);
    NotifyStateChanged(StateField::Recording);

    std::printf("[StateManager] Recording STARTED at t=%.3f\n",
               state_.currentEEGTime());

    // TODO: Initialize file writer here when hardware integration is complete

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::StopRecording() {
    if (!state_.isRecordingToFile) {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    // Stop recording
    state_.isRecordingToFile = false;
    state_.isPaused = false;

    LogStateChange("recording", true, false);
    NotifyStateChanged(StateField::Recording);

    std::printf("[StateManager] Recording STOPPED at t=%.3f (duration: %.3f s)\n",
               state_.currentEEGTime(),
               state_.currentEEGTime() - state_.recordingStartTime);

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

    LogStateChange("paused", false, true);
    NotifyStateChanged(StateField::Paused);

    std::printf("[StateManager] Recording PAUSED at t=%.3f\n",
               state_.currentEEGTime());

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

    LogStateChange("paused", true, false);
    NotifyStateChanged(StateField::Paused);

    std::printf("[StateManager] Recording RESUMED at t=%.3f\n",
               state_.currentEEGTime());

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

    // Validation: must have at least 1 channel
    if (channels.empty()) {
        return {StateChangeResult::ValidationFailed,
                "Must select at least one channel"};
    }

    // Validation: check channel count limits (64-136 from requirements)
    if (channels.size() > 136) {
        return {StateChangeResult::ValidationFailed,
                "Maximum 136 channels supported"};
    }

    // Perform state change
    std::string oldGroup = state_.currentChannelGroupName;
    size_t oldCount = state_.selectedChannels.size();

    state_.currentChannelGroupName = groupName;
    state_.selectedChannels = channels;

    // Impedance check is invalidated when channels change
    impedanceCheckPassed_ = false;

    LogStateChange("channel_group", oldGroup, groupName);
    std::printf("[StateManager] Channel configuration changed: %zu -> %zu channels\n",
               oldCount, channels.size());

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
    state_.lastWinIdx = oldIdx; // Track for smooth transitions

    LogStateChange("window_size", oldIdx, windowIndex);
    NotifyStateChanged(StateField::DisplayWindow);

    std::printf("[StateManager] Display window changed: %.1f -> %.1f seconds\n",
               WINDOW_OPTIONS[oldIdx], WINDOW_OPTIONS[windowIndex]);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::SetDisplayAmplitude(int amplitudeIndex) {
    std::string errorMsg;
    if (!ValidateAmplitudeIndex(amplitudeIndex, errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    int oldIdx = state_.ampIdx;
    state_.ampIdx = amplitudeIndex;

    LogStateChange("amplitude_scale", oldIdx, amplitudeIndex);
    NotifyStateChanged(StateField::DisplayAmplitude);

    std::printf("[StateManager] Amplitude scale changed: %d -> %d µV\n",
               AMP_PP_UV_OPTIONS[oldIdx], AMP_PP_UV_OPTIONS[amplitudeIndex]);

    return {StateChangeResult::Success, ""};
}

// ===== NOISE/ARTIFACT CONTROL =====

StateChangeError AppStateManager::SetNoiseScale(float scale) {
    std::string errorMsg;
    if (!ValidateScale(scale, "noise_scale", errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    float oldScale = state_.noiseScale;
    state_.noiseScale = scale;

    LogStateChange("noise_scale", oldScale, scale);
    NotifyStateChanged(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::SetArtifactScale(float scale) {
    std::string errorMsg;
    if (!ValidateScale(scale, "artifact_scale", errorMsg)) {
        return {StateChangeResult::ValidationFailed, errorMsg};
    }

    float oldScale = state_.artifactScale;
    state_.artifactScale = scale;

    LogStateChange("artifact_scale", oldScale, scale);
    NotifyStateChanged(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

// ===== OBSERVER PATTERN =====

void AppStateManager::AddObserver(StateObserver observer) {
    observers_.push_back(observer);
    std::printf("[StateManager] Observer registered (total: %zu)\n", observers_.size());
}

void AppStateManager::ClearObservers() {
    observers_.clear();
    std::printf("[StateManager] All observers cleared\n");
}

void AppStateManager::NotifyStateChanged(StateField field) {
    for (auto& observer : observers_) {
        observer(field);
    }
}

// ===== AUDIT LOG =====

void AppStateManager::EnableAuditLog(bool enable, const std::string& logFilePath) {
    auditLogEnabled_ = enable;

    if (enable) {
        // Close existing file if open
        if (auditLogFile_.is_open()) {
            auditLogFile_.close();
        }

        // Determine log file path
        if (logFilePath.empty()) {
            auditLogPath_ = "elda_audit_log.txt";
        } else {
            auditLogPath_ = logFilePath;
        }

        // Open log file in append mode
        auditLogFile_.open(auditLogPath_, std::ios::app);

        if (auditLogFile_.is_open()) {
            std::printf("[StateManager] Audit log enabled: %s\n", auditLogPath_.c_str());
            WriteToAuditLog("=== AUDIT LOG SESSION STARTED ===");
        } else {
            std::fprintf(stderr, "[StateManager] ERROR: Failed to open audit log: %s\n",
                       auditLogPath_.c_str());
            auditLogEnabled_ = false;
        }
    } else {
        if (auditLogFile_.is_open()) {
            WriteToAuditLog("=== AUDIT LOG SESSION ENDED ===");
            auditLogFile_.close();
        }
        std::printf("[StateManager] Audit log disabled\n");
    }
}

// ===== VALIDATION HELPERS =====

bool AppStateManager::ValidateCanStartMonitoring(std::string& errorMsg) {
    // Check that channels are configured
    if (state_.selectedChannels.empty()) {
        errorMsg = "No channels selected - configure channels before monitoring";
        return false;
    }

    // TODO: Add hardware connectivity check when SDK is integrated
    // if (!hardwareConnected) {
    //     errorMsg = "Hardware not connected";
    //     return false;
    // }

    return true;
}

bool AppStateManager::ValidateCanStartRecording(std::string& errorMsg) {
    // Must be monitoring
    if (!state_.isMonitoring) {
        errorMsg = "Cannot record while not monitoring";
        return false;
    }

    // Must have channels selected
    if (state_.selectedChannels.empty()) {
        errorMsg = "No channels selected";
        return false;
    }

    // Impedance check must have passed (per IEC 60601-2-26 requirements)
    if (!impedanceCheckPassed_) {
        errorMsg = "Impedance check required - all channels must be <50kΩ";
        return false;
    }

    return true;
}

bool AppStateManager::ValidateCanChangeChannels(std::string& errorMsg) {
    // Cannot change channels during active recording
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

// ===== LOGGING HELPERS =====

void AppStateManager::LogStateChange(const std::string& field,
                                     const std::string& oldValue,
                                     const std::string& newValue) {
    std::string logMsg = "[StateChange] " + field + ": '" + oldValue +
                        "' -> '" + newValue + "' at t=" +
                        std::to_string(state_.currentEEGTime()) + "s";

    std::printf("%s\n", logMsg.c_str());
    WriteToAuditLog(logMsg);
}

void AppStateManager::LogStateChange(const std::string& field, bool oldValue, bool newValue) {
    LogStateChange(field,
                  oldValue ? std::string("true") : std::string("false"),
                  newValue ? std::string("true") : std::string("false"));
}

void AppStateManager::LogStateChange(const std::string& field, int oldValue, int newValue) {
    LogStateChange(field, std::to_string(oldValue), std::to_string(newValue));
}

void AppStateManager::LogStateChange(const std::string& field, float oldValue, float newValue) {
    char oldBuf[32], newBuf[32];
    std::snprintf(oldBuf, sizeof(oldBuf), "%.3f", oldValue);
    std::snprintf(newBuf, sizeof(newBuf), "%.3f", newValue);
    LogStateChange(field, std::string(oldBuf), std::string(newBuf));
}

void AppStateManager::WriteToAuditLog(const std::string& message) {
    if (auditLogEnabled_ && auditLogFile_.is_open()) {
        auditLogFile_ << GetTimestamp() << " | " << message << std::endl;
        auditLogFile_.flush(); // Ensure immediate write for regulatory compliance
    }
}

std::string AppStateManager::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

} // namespace elda