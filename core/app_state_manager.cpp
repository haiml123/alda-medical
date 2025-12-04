#include "app_state_manager.h"

#include <unordered_map>

namespace elda
{
// ===== CONSTRUCTOR / DESTRUCTOR =====

AppStateManager::AppStateManager(AppState& state) : state_(state), impedance_check_passed_(false)
{
}

AppStateManager::~AppStateManager()
{
}

// ===== MONITORING CONTROL =====

StateChangeError AppStateManager::set_monitoring(bool enable)
{
    if (state_.is_monitoring == enable)
    {
        return {StateChangeResult::Success, ""};
    }

    if (enable)
    {
        std::string error_msg;
        if (!validate_can_start_monitoring(error_msg))
        {
            return {StateChangeResult::ValidationFailed, error_msg};
        }

        state_.is_monitoring = true;
        notify_state_changed(StateField::Monitoring);
    }
    else
    {
        if (state_.is_recording_to_file)
        {
            auto result = stop_recording();
            // Continue stopping monitoring even if recording stop fails
        }

        state_.is_monitoring = false;
        notify_state_changed(StateField::Monitoring);
    }

    return {StateChangeResult::Success, ""};
}

// ===== RECORDING CONTROL =====

StateChangeError AppStateManager::start_recording()
{
    if (state_.is_recording_to_file)
    {
        return {StateChangeResult::InvalidTransition, "Already recording"};
    }

    state_.recording_state = RecordingState::Recording;

    std::string error_msg;
    if (!validate_can_start_recording(error_msg))
    {
        if (!impedance_check_passed_)
        {
            return {StateChangeResult::ImpedanceCheckRequired, error_msg};
        }
        return {StateChangeResult::ValidationFailed, error_msg};
    }

    state_.is_recording_to_file = true;
    state_.recording_start_time = state_.current_eeg_time();

    notify_state_changed(StateField::Recording);

    // TODO: Initialize file writer here when hardware integration is complete

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::stop_recording()
{
    if (!state_.is_recording_to_file)
    {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    state_.recording_state = RecordingState::None;
    state_.is_recording_to_file = false;

    notify_state_changed(StateField::Recording);

    // TODO: Finalize and close file writer

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::pause_recording()
{
    if (!state_.is_recording_to_file)
    {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    if (state_.is_paused)
    {
        return {StateChangeResult::InvalidTransition, "Already paused"};
    }

    state_.recording_state = RecordingState::Paused;

    state_.pause_marks.push_back({state_.current_eeg_time()});

    notify_state_changed(StateField::Paused);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::resume_recording()
{
    if (!state_.is_recording_to_file)
    {
        return {StateChangeResult::InvalidTransition, "Not currently recording"};
    }

    state_.recording_state = RecordingState::Recording;

    if (!state_.is_paused)
    {
        return {StateChangeResult::InvalidTransition, "Not currently paused"};
    }

    notify_state_changed(StateField::Paused);

    return {StateChangeResult::Success, ""};
}

// ===== DISPLAY SETTINGS =====

StateChangeError AppStateManager::set_display_window(int windowIndex)
{
    std::string error_msg;
    if (!validate_window_index(windowIndex, error_msg))
    {
        return {StateChangeResult::ValidationFailed, error_msg};
    }

    int old_idx = state_.win_idx;
    state_.win_idx = windowIndex;
    state_.last_win_idx = old_idx;

    notify_state_changed(StateField::DisplayWindow);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::set_display_amplitude(int amplitude_index)
{
    std::string error_msg;
    if (!validate_amplitude_index(amplitude_index, error_msg))
    {
        return {StateChangeResult::ValidationFailed, error_msg};
    }

    state_.amp_idx = amplitude_index;

    notify_state_changed(StateField::DisplayAmplitude);

    return {StateChangeResult::Success, ""};
}

// ===== NOISE/ARTIFACT CONTROL =====

StateChangeError AppStateManager::set_noise_scale(float scale)
{
    std::string error_msg;
    if (!validate_scale(scale, "noise_scale", error_msg))
    {
        return {StateChangeResult::ValidationFailed, error_msg};
    }

    state_.noise_scale = scale;

    notify_state_changed(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

StateChangeError AppStateManager::set_artifact_scale(float scale)
{
    std::string error_msg;
    if (!validate_scale(scale, "artifact_scale", error_msg))
    {
        return {StateChangeResult::ValidationFailed, error_msg};
    }

    state_.artifact_scale = scale;

    notify_state_changed(StateField::NoiseSettings);

    return {StateChangeResult::Success, ""};
}

std::vector<const models::Channel*>& AppStateManager::get_selected_channels() const
{
    return state_.selected_channels;
}

StateChangeError AppStateManager::set_active_channel_group(const models::ChannelsGroup& group)
{
    std::string error_msg;
    if (!validate_can_change_channels(error_msg))
    {
        return {StateChangeResult::InvalidTransition, error_msg};
    }
    if (group.get_channel_count() == 0)
    {
        return {StateChangeResult::ValidationFailed, "Selected group has no channels"};
    }

    std::unordered_map<std::string, const models::Channel*> by_id;

    const auto* avail = state_.available_channels;
    if (!avail)
    {
        return {StateChangeResult::ValidationFailed, "No channels available"};
    }

    by_id.reserve(avail->size());
    for (const auto& ch : *avail)
    {
        by_id.emplace(ch.id, &ch);
    }

    std::vector<const models::Channel*> new_selected_ptrs;
    new_selected_ptrs.reserve(group.get_channel_count());
    for (const auto& id : group.channel_ids)
    {
        if (auto it = by_id.find(id); it != by_id.end())
        {
            new_selected_ptrs.push_back(it->second);
        }
    }

    if (new_selected_ptrs.empty())
    {
        return {StateChangeResult::ValidationFailed, "No valid channels found in group"};
    }

    state_.current_channel_group_name = group.name;
    state_.selected_channels = std::move(new_selected_ptrs);
    impedance_check_passed_ = true;

    notify_state_changed(StateField::ChannelConfig);
    return {StateChangeResult::Success, ""};
}

// ===== OBSERVER PATTERN =====

AppStateManager::ObserverHandle AppStateManager::add_observer(StateObserver observer)
{
    ObserverHandle handle = next_handle_++;
    observers_.push_back({handle, observer});
    return handle;
}

void AppStateManager::remove_observer(ObserverHandle handle)
{
    auto it = std::remove_if(observers_.begin(),
                             observers_.end(),
                             [handle](const auto& pair)
                             {
                                 return pair.first == handle;
                             });
    observers_.erase(it, observers_.end());
}

void AppStateManager::clear_observers()
{
    observers_.clear();
}

void AppStateManager::notify_state_changed(StateField field)
{
    for (const auto& [handle, observer] : observers_)
    {
        observer(field);
    }
}

// ===== VALIDATION HELPERS =====

bool AppStateManager::validate_can_start_monitoring(std::string& error_msg)
{
    if (state_.selected_channels.empty())
    {
        error_msg = "No channels selected - configure channels before monitoring";
        return false;
    }

    // TODO: Add hardware connectivity check when SDK is integrated

    return true;
}

bool AppStateManager::validate_can_start_recording(std::string& error_msg)
{
    if (!state_.is_monitoring)
    {
        error_msg = "Cannot record while not monitoring";
        return false;
    }

    if (state_.selected_channels.empty())
    {
        error_msg = "No channels selected";
        return false;
    }

    if (!impedance_check_passed_)
    {
        error_msg = "Impedance check required - all channels must be <50kΩ";
    }

    return true;
}

bool AppStateManager::validate_can_change_channels(std::string& error_msg)
{
    if (state_.is_recording_to_file && !state_.is_paused)
    {
        error_msg = "Cannot change channel configuration during active recording";
        return false;
    }

    return true;
}

bool AppStateManager::validate_window_index(int index, std::string& error_msg)
{
    if (index < 0 || index >= WINDOW_COUNT)
    {
        error_msg = "Invalid window index (must be 0-" + std::to_string(WINDOW_COUNT - 1) + ")";
        return false;
    }
    return true;
}

bool AppStateManager::validate_amplitude_index(int index, std::string& error_msg)
{
    if (index < 0 || index >= AMP_COUNT)
    {
        error_msg = "Invalid amplitude index (must be 0-" + std::to_string(AMP_COUNT - 1) + ")";
        return false;
    }
    return true;
}

bool AppStateManager::validate_scale(float scale, const std::string& param_name, std::string& error_msg)
{
    if (scale < 0.0f || scale > 5.0f)
    {
        error_msg = param_name + " must be between 0.0 and 5.0";
        return false;
    }
    return true;
}
}  // namespace elda
