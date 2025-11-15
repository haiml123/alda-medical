#include "monitoring_model.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace elda::views::monitoring {

MonitoringModel::MonitoringModel(AppState& state, elda::AppStateManager& state_manager)
    : models::MVPBaseModel(state_manager), state_(state), state_manager_(state_manager) {
    initialize_buffers();
}

void MonitoringModel::initialize_buffers() {
    chart_data_.num_channels = CHANNELS;
    chart_data_.sample_rate_hz = (int)SAMPLE_RATE_HZ;
    chart_data_.buffer_size = kBufferSize;
    chart_data_.amplitude_pp_uv = state_.amp_pp_uv();
    chart_data_.window_seconds = state_.window_sec();
    chart_data_.gain_multiplier = state_.gain_mul();
    chart_data_.playhead_seconds = 0.0;

    chart_data_.ring.t_abs.clear();
    chart_data_.ring.data.clear();
    chart_data_.ring.write = 0;
    chart_data_.ring.filled = false;
}

void MonitoringModel::start_acquisition() {
    std::cout << "[Model] Start acquisition" << std::endl;
}

void MonitoringModel::stop_acquisition() {
    std::cout << "[Model] Stop acquisition" << std::endl;
}

void MonitoringModel::update(float delta_time) {
    if (state_manager_.is_monitoring()) {
        state_.tick_display(true);
        update_chart_data();
        generate_synthetic_data(delta_time);
    } else {
        state_.tick_display(false);
    }
}

void MonitoringModel::generate_synthetic_data(float /*delta_time*/) {
    static SynthEEG synth_gen;
    std::vector<float> sample(CHANNELS);

    int samples_this_frame = state_.sampler.due();

    for (int i = 0; i < samples_this_frame; ++i) {
        synth_gen.next(sample);

        for (int ch = 0; ch < CHANNELS; ++ch) {
            sample[ch] *= state_.noise_scale;
        }

        state_.ring.push(sample);
    }
}

void MonitoringModel::update_chart_data() {
    chart_data_.amplitude_pp_uv = state_.amp_pp_uv();
    chart_data_.window_seconds = state_.window_sec();
    chart_data_.gain_multiplier = state_.gain_mul();
    chart_data_.playhead_seconds = state_.ring.now;

    chart_data_.ring.t_abs.resize(BUFFER_SIZE);
    chart_data_.ring.data.resize(CHANNELS);

    for (int i = 0; i < BUFFER_SIZE; ++i) {
        chart_data_.ring.t_abs[i] = static_cast<double>(state_.ring.t_abs[i]);
    }

    for (int ch = 0; ch < CHANNELS; ++ch) {
        chart_data_.ring.data[ch].resize(BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; ++i) {
            chart_data_.ring.data[ch][i] = static_cast<double>(state_.ring.data[ch][i]);
        }
    }

    chart_data_.ring.write = state_.ring.write;
    chart_data_.ring.filled = state_.ring.filled;
    chart_data_.buffer_size = BUFFER_SIZE;
}

// ============================================================================
// ACTIONS
// ============================================================================

void MonitoringModel::toggle_monitoring() const {
    const bool monitoring = state_manager_.is_monitoring();
    std::printf("[Model] ToggleMonitoring - current: %s\n",
               monitoring ? "MONITORING" : "IDLE");

    auto result = state_manager_.set_monitoring(!monitoring);
    if (!result.is_success()) {
        std::fprintf(stderr, "[Model] Monitor toggle failed: %s\n", result.message.c_str());
    } else {
        std::printf("[Model] Monitor toggle SUCCESS - new: %s\n",
                   !monitoring ? "MONITORING" : "IDLE");

        if (!monitoring) {
            std::printf("[Model] Resetting ring buffer\n");
            state_.ring.reset();
            state_.playhead_seconds = 0.0;
            state_.sampler = SampleClock(SAMPLE_RATE_HZ);
        }
    }
}

void MonitoringModel::stop_recording() const {
    state_manager_.stop_recording();
}

void MonitoringModel::toggle_recording() const {
    const bool recording_active = state_manager_.is_recording();
    const bool currently_paused = state_manager_.is_paused();
    std::fprintf(stderr, "[Model] Monitor toggle failed: %hhd\n", recording_active, currently_paused);
    if (recording_active && !currently_paused) {
        auto result = state_manager_.pause_recording();
        if (!result.is_success()) {
            std::fprintf(stderr, "[Model] Pause failed: %s\n", result.message.c_str());
        }
    } else {
        StateChangeError result;
        if (currently_paused) {
            result = state_manager_.resume_recording();
        } else {
            result = state_manager_.start_recording();
            if (result.result == StateChangeResult::ImpedanceCheckRequired) {
                std::fprintf(stderr, "[Model] Recording requires impedance check first\n");
            }
        }

        if (!result.is_success()) {
            std::fprintf(stderr, "[Model] Record toggle failed: %s\n", result.message.c_str());
        }
    }
}

void MonitoringModel::increase_window() const {
    if (state_.win_idx < WINDOW_COUNT - 1) {
        state_manager_.set_display_window(state_.win_idx + 1);
    }
}

void MonitoringModel::decrease_window() const {
    if (state_.win_idx > 0) {
        state_manager_.set_display_window(state_.win_idx - 1);
    }
}

void MonitoringModel::increase_amplitude() const {
    if (state_.amp_idx < AMP_COUNT - 1) {
        state_manager_.set_display_amplitude(state_.amp_idx + 1);
    }
}

void MonitoringModel::decrease_amplitude() const {
    if (state_.amp_idx > 0) {
        state_manager_.set_display_amplitude(state_.amp_idx - 1);
    }
}

void MonitoringModel::refresh_available_groups() const {
    auto& service = services::ChannelManagementService::get_instance();
    state_.available_groups = service.get_all_channel_groups();

    std::printf("[MonitoringModel] ✓ Available groups refreshed, total: %zu\n",
               state_.available_groups.size());
}

void MonitoringModel::on_group_selected(const models::ChannelsGroup& group) const {
    state_manager_.set_active_channel_group(group);
}

std::vector<const models::Channel *>& MonitoringModel::get_selected_channels() const {
    return state_manager_.get_selected_channels();
}

void MonitoringModel::apply_channel_configuration(const elda::models::ChannelsGroup& group) const {
    refresh_available_groups();
}

// ============================================================================
// TAB BAR SUPPORT
// ============================================================================

int MonitoringModel::get_active_group_index() const {
    const std::string& active_name = state_.current_channel_group_name;
    const auto& groups = state_.available_groups;

    auto it = std::find_if(groups.begin(), groups.end(),
        [&active_name](const elda::models::ChannelsGroup& g) {
            return g.name == active_name;
        });

    if (it != groups.end()) {
        return std::distance(groups.begin(), it);
    }

    if (!active_name.empty()) {
        std::fprintf(stderr, "[Model] Warning: Active group '%s' not found\n",
                    active_name.c_str());
    }
    return 0;
}

} // namespace elda
