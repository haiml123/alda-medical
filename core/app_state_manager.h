#pragma once

#include "core/core.h"
#include "models/channel.h"
#include <functional>
#include <string>
#include <vector>

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
    // Observer handle type for registration/removal
    using ObserverHandle = size_t;
    using StateObserver = std::function<void(StateField)>;

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
     * Get current window index
     */
    int GetWindowIndex() const { return state_.winIdx; }

    /**
     * Get current amplitude index
     */
    int GetAmplitudeIndex() const { return state_.ampIdx; }

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

    /**
     * Register an observer for state changes
     * Observer will be called whenever state changes with the field that changed
     * @param observer Callback function
     * @return Handle to use for removing the observer
     */
    ObserverHandle AddObserver(StateObserver observer);

    /**
     * Remove a specific observer by handle
     * @param handle Handle returned from AddObserver
     */
    void RemoveObserver(ObserverHandle handle);

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

private:
    // === VALIDATION HELPERS ===

    bool ValidateCanStartMonitoring(std::string& errorMsg);
    bool ValidateCanStartRecording(std::string& errorMsg);
    bool ValidateCanChangeChannels(std::string& errorMsg);
    bool ValidateWindowIndex(int index, std::string& errorMsg);
    bool ValidateAmplitudeIndex(int index, std::string& errorMsg);
    bool ValidateScale(float scale, const std::string& paramName, std::string& errorMsg);

    // === OBSERVER NOTIFICATION ===

    void NotifyStateChanged(StateField field);

    // === MEMBERS ===

    AppState& state_;                                           // Reference to actual app state
    std::vector<std::pair<ObserverHandle, StateObserver>> observers_;  // Registered observers with handles
    ObserverHandle nextHandle_{0};                              // Next observer handle to assign
    bool impedanceCheckPassed_{false};                          // Impedance validation flag
};

} // namespace elda