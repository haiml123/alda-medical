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
    bool is_success() const { return result == StateChangeResult::Success; }
    explicit operator bool() const { return is_success(); }
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
    StateChangeError set_monitoring(bool enable);

    /**
     * Check if monitoring is currently active
     */
    bool is_monitoring() const { return state_.is_monitoring; }

    // === RECORDING CONTROL ===

    /**
     * Start recording to file
     * Validates: monitoring must be active, channels must be selected, impedance must be valid
     * @return Result of state change
     */
    StateChangeError start_recording();

    /**
     * Stop recording to file
     * @return Result of state change
     */
    StateChangeError stop_recording();

    /**
     * Pause recording (keeps monitoring active)
     * @return Result of state change
     */
    StateChangeError pause_recording();

    /**
     * Resume recording from pause
     * @return Result of state change
     */
    StateChangeError resume_recording();

    bool is_recording_active() const { return state_.is_recording_to_file; }

    /**
     * Check if recording is currently active
     */
    bool is_recording() const { return state_.recording_state == RecordingState::Recording; }

    RecordingState get_recording_state() const { return state_.recording_state; }
    /**
     * Check if recording is paused
     */
    bool is_paused() const { return state_.recording_state == RecordingState::Paused; }

    bool is_stopped() const { return !state_.is_recording_to_file && state_.recording_state == RecordingState::None; }

    // === CHANNEL CONFIGURATION ===

    /**
     * Set active channel group and load its channels
     * Updates both the group name and selected channels list
     * @param group The channel group to activate
     * @return Result of state change
     */
    StateChangeError set_active_channel_group(const models::ChannelsGroup& group);

    /**
     * Get current channel configuration name
     */
    std::string get_channel_group_name() const { return state_.current_channel_group_name; }

    /**
     * Get number of selected channels
     */
    size_t get_selected_channel_count() const { return state_.selected_channels.size(); }

    // === DISPLAY SETTINGS ===

    /**
     * Set display window size
     * @param windowIndex Index into WINDOW_OPTIONS array (0-4)
     * @return Result of state change
     */
    StateChangeError set_display_window(int windowIndex);

    /**
     * Set display amplitude scale
     * @param amplitudeIndex Index into AMP_PP_UV_OPTIONS array (0-6)
     * @return Result of state change
     */
    StateChangeError set_display_amplitude(int amplitudeIndex);

    /**
     * Get current window index
     */
    int get_window_index() const { return state_.win_idx; }

    /**
     * Get current amplitude index
     */
    int get_amplitude_index() const { return state_.amp_idx; }

    /**
     * Get current window size in seconds
     */
    float get_window_seconds() const { return state_.window_sec(); }

    /**
     * Get current amplitude scale in microvolts
     */
    int get_amplitude_micro_volts() const { return state_.amp_pp_uv(); }

    // === NOISE/ARTIFACT CONTROL ===

    /**
     * Set noise scale multiplier
     * @param scale Multiplier (0.0 to 5.0)
     * @return Result of state change
     */
    StateChangeError set_noise_scale(float scale);

    /**
     * Set artifact scale multiplier
     * @param scale Multiplier (0.0 to 5.0)
     * @return Result of state change
     */
    StateChangeError set_artifact_scale(float scale);

    std::vector<const models::Channel *>& get_selected_channels() const;

    // === READ-ONLY STATE ACCESS ===

    /**
     * Get const reference to underlying app state
     * Use this for reading state in UI components
     */
    const AppState& get_state() const { return state_; }

    // === OBSERVER PATTERN ===

    /**
     * Register an observer for state changes
     * Observer will be called whenever state changes with the field that changed
     * @param observer Callback function
     * @return Handle to use for removing the observer
     */
    ObserverHandle add_observer(StateObserver observer);

    /**
     * Remove a specific observer by handle
     * @param handle Handle returned from AddObserver
     */
    void remove_observer(ObserverHandle handle);

    /**
     * Clear all observers
     */
    void clear_observers();

    // === IMPEDANCE VALIDATION ===

    /**
     * Set whether impedance check has been performed
     * @param passed True if all channels passed impedance check (<50kΩ)
     */
    void set_impedance_check_passed(bool passed) { impedance_check_passed_ = passed; }

    /**
     * Check if impedance validation has passed
     */
    bool is_impedance_check_passed() const { return impedance_check_passed_; }

private:
    // === VALIDATION HELPERS ===

    bool validate_can_start_monitoring(std::string& error_msg);
    bool validate_can_start_recording(std::string& error_msg);
    bool validate_can_change_channels(std::string& error_msg);
    bool validate_window_index(int index, std::string& error_msg);
    bool validate_amplitude_index(int index, std::string& error_msg);
    bool validate_scale(float scale, const std::string& param_name, std::string& error_msg);

    // === OBSERVER NOTIFICATION ===

    void notify_state_changed(StateField field);

    // === MEMBERS ===

    AppState& state_;                                           // Reference to actual app state
    std::vector<std::pair<ObserverHandle, StateObserver>> observers_;  // Registered observers with handles
    ObserverHandle next_handle_{0};                              // Next observer handle to assign
    bool impedance_check_passed_{false};                          // Impedance validation flag
};

} // namespace elda
