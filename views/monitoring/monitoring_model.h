#ifndef ELDA_MONITORING_MODEL_H
#define ELDA_MONITORING_MODEL_H

#include "UI/chart/chart_data.h"
#include "core/app_state_manager.h"
#include "core/core.h"
#include "models/channels_group.h"
#include "models/mvp_base_model.h"
#include "services/channel_management_service.h"

namespace elda::views::monitoring
{

class MonitoringModel : public models::MVPBaseModel
{
  public:
    MonitoringModel(AppState& state, elda::AppStateManager& state_manager);
    ~MonitoringModel() = default;

    // Lifecycle
    void start_acquisition();
    void stop_acquisition();
    void update(float delta_time);

    // Actions (called by Presenter)
    void toggle_monitoring() const;

    void stop_recording() const;

    void toggle_recording() const;
    void increase_window() const;
    void decrease_window() const;
    void increase_amplitude() const;
    void decrease_amplitude() const;
    void apply_channel_configuration(const elda::models::ChannelsGroup& group) const;

    void refresh_available_groups() const;

    void on_group_selected(const models::ChannelsGroup& group) const;

    std::vector<const models::Channel*>& get_selected_channels() const;

    // Getters (Presenter collects this data for View)
    const ChartData& get_chart_data() const
    {
        return chart_data_;
    }

    bool is_monitoring() const
    {
        return state_manager_.is_monitoring();
    }

    bool can_record() const
    {
        return is_monitoring();
    }

    bool is_recording_active() const
    {
        return state_manager_.is_recording_active() && state_manager_.is_monitoring();
    }

    RecordingState get_recording_state() const
    {
        return state_manager_.get_recording_state();
    }

    bool is_currently_paused() const
    {
        return state_manager_.is_paused() && state_manager_.is_monitoring();
    }
    bool is_currently_recording() const
    {
        return state_manager_.is_recording() && state_manager_.is_monitoring();
    }
    bool is_stopped() const
    {
        return state_manager_.is_stopped();
    }
    int get_window_seconds() const
    {
        return (int)state_manager_.get_window_seconds();
    }
    int get_amplitude_micro_volts() const
    {
        return state_manager_.get_amplitude_micro_volts();
    }
    double get_sample_rate_hz() const
    {
        return SAMPLE_RATE_HZ;
    }

    const std::vector<models::ChannelsGroup>& get_available_groups() const
    {
        return state_.available_groups;
    }

    int get_active_group_index() const;

  private:
    AppState& state_;
    AppStateManager& state_manager_;
    ChartData chart_data_;
    static constexpr int kBufferSize = 25000;

    void initialize_buffers();
    void generate_synthetic_data(float delta_time);
    void update_chart_data();
};

}  // namespace elda::views::monitoring

#endif  // ELDA_MONITORING_MODEL_H
