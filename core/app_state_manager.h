#pragma once

#include "core/core.h"
#include "models/channel.h"
#include <functional>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>

namespace elda {

// ===== Result Types =====

enum class StateChangeResult {
    Success,
    ValidationFailed,
    HardwareError,
    InvalidTransition,
    ImpedanceCheckRequired
};

struct StateChangeError {
    StateChangeResult result;
    std::string message;

    // Helper methods
    bool IsSuccess() const { return result == StateChangeResult::Success; }
    explicit operator bool() const { return IsSuccess(); }
};

// ===== State Field Enum for Observers =====

enum class StateField {
    Monitoring,
    Recording,
    Paused,
    ChannelConfig,
    DisplayWindow,
    DisplayAmplitude,
    NoiseSettings
};

// ===== App State Manager =====

class AppStateManager {
public:
    // Constructor
    explicit AppStateManager(AppState& state);
    ~AppStateManager();

    // === MONITORING CONTROL ===

    /**
     * Start or stop monitoring mode
     * When stopping, automatically stops recording if active
     * @param enable True to start monitoring, false to stop
     * @return Result of state change
     */
    StateChangeError SetMonitoring(bool enable);

    /**
     * Check if monitoring is currently active
     */
    bool IsMonitoring() const { return state_.isMonitoring; }

    // === RECORDING CONTROL ===

    /**
     * Start recording to file
     * Validates: monitoring must be active, channels must be selected, impedance must be valid
     * @return Result of state change
     */
    StateChangeError StartRecording();

    /**
     * Stop recording to file
     * @return Result of state change
     */
    StateChangeError StopRecording();

    /**
     * Pause recording (keeps monitoring active)
     * @return Result of state change
     */
    StateChangeError PauseRecording();

    /**
     * Resume recording from pause
     * @return Result of state change
     */
    StateChangeError ResumeRecording();

    /**
     * Check if recording is currently active
     */
    bool IsRecording() const { return state_.isRecordingToFile; }

    /**
     * Check if recording is paused
     */
    bool IsPaused() const { return state_.isPaused; }

    // === CHANNEL CONFIGURATION ===

    /**
     * Set channel configuration
     * Validates: cannot change during active recording, must have at least 1 channel
     * @param groupName Name of the channel group
     * @param channels Vector of selected channels
     * @return Result of state change
     */
    StateChangeError SetChannelConfiguration(
        const std::string& groupName,
        const std::vector<models::Channel>& channels
    );

    /**
     * Get current channel configuration name
     */
    std::string GetChannelGroupName() const { return state_.currentChannelGroupName; }

    /**
     * Get number of selected channels
     */
    size_t GetSelectedChannelCount() const { return state_.selectedChannels.size(); }

    // === DISPLAY SETTINGS ===

    /**
     * Set display window size
     * @param windowIndex Index into WINDOW_OPTIONS array (0-4)
     * @return Result of state change
     */
    StateChangeError SetDisplayWindow(int windowIndex);

    /**
     * Set display amplitude scale
     * @param amplitudeIndex Index into AMP_PP_UV_OPTIONS array (0-6)
     * @return Result of state change
     */
    StateChangeError SetDisplayAmplitude(int amplitudeIndex);

    /**
     * Get current window size in seconds
     */
    float GetWindowSeconds() const { return state_.windowSec(); }

    /**
     * Get current amplitude scale in microvolts
     */
    int GetAmplitudeMicroVolts() const { return state_.ampPPuV(); }

    // === NOISE/ARTIFACT CONTROL ===

    /**
     * Set noise scale multiplier
     * @param scale Multiplier (0.0 to 5.0)
     * @return Result of state change
     */
    StateChangeError SetNoiseScale(float scale);

    /**
     * Set artifact scale multiplier
     * @param scale Multiplier (0.0 to 5.0)
     * @return Result of state change
     */
    StateChangeError SetArtifactScale(float scale);

    // === READ-ONLY STATE ACCESS ===

    /**
     * Get const reference to underlying app state
     * Use this for reading state in UI components
     */
    const AppState& GetState() const { return state_; }

    // === OBSERVER PATTERN ===

    using StateObserver = std::function<void(StateField)>;

    /**
     * Register an observer for state changes
     * Observer will be called whenever state changes with the field that changed
     */
    void AddObserver(StateObserver observer);

    /**
     * Clear all observers
     */
    void ClearObservers();

    // === IMPEDANCE VALIDATION ===

    /**
     * Set whether impedance check has been performed
     * @param passed True if all channels passed impedance check (<50kΩ)
     */
    void SetImpedanceCheckPassed(bool passed) { impedanceCheckPassed_ = passed; }

    /**
     * Check if impedance validation has passed
     */
    bool IsImpedanceCheckPassed() const { return impedanceCheckPassed_; }

    // === AUDIT LOG ===

    /**
     * Enable or disable audit logging to file
     * @param enable True to enable, false to disable
     * @param logFilePath Path to log file (uses default if empty)
     */
    void EnableAuditLog(bool enable, const std::string& logFilePath = "");

    /**
     * Get current audit log file path
     */
    std::string GetAuditLogPath() const { return auditLogPath_; }

private:
    // === VALIDATION HELPERS ===

    bool ValidateCanStartMonitoring(std::string& errorMsg);
    bool ValidateCanStartRecording(std::string& errorMsg);
    bool ValidateCanChangeChannels(std::string& errorMsg);
    bool ValidateWindowIndex(int index, std::string& errorMsg);
    bool ValidateAmplitudeIndex(int index, std::string& errorMsg);
    bool ValidateScale(float scale, const std::string& paramName, std::string& errorMsg);

    // === LOGGING ===

    void LogStateChange(const std::string& field, const std::string& oldValue,
                       const std::string& newValue);
    void LogStateChange(const std::string& field, bool oldValue, bool newValue);
    void LogStateChange(const std::string& field, int oldValue, int newValue);
    void LogStateChange(const std::string& field, float oldValue, float newValue);

    void WriteToAuditLog(const std::string& message);
    std::string GetTimestamp() const;

    // === OBSERVER NOTIFICATION ===

    void NotifyStateChanged(StateField field);

    // === MEMBERS ===

    AppState& state_;                           // Reference to actual app state
    std::vector<StateObserver> observers_;      // Registered observers
    bool impedanceCheckPassed_{false};          // Impedance validation flag

    // Audit logging
    bool auditLogEnabled_{false};
    std::string auditLogPath_;
    std::ofstream auditLogFile_;
};

} // namespace elda