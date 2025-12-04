#pragma once
#include "UI/chart/chart_data.h"
#include "UI/tabbar/tabbar.h"
#include "core/core.h"
#include "models/channel.h"
#include "models/channels_group.h"

#include <functional>
#include <vector>

namespace elda::views::monitoring
{

/**
 * All data the view needs to display
 * Presenter collects this from Model
 */
struct MonitoringViewData
{
    // Chart
    const ChartData* chart_data = nullptr;

    // Toolbar state
    bool monitoring = false;
    bool can_record = false;
    bool recording_active = false;
    bool currently_recording = false;
    bool currently_paused = false;
    int window_seconds = 10;
    int amplitude_micro_volts = 100;
    double sample_rate_hz = 1000.0;
    RecordingState recording_state;

    // Tab bar
    const std::vector<elda::models::ChannelsGroup>* groups = nullptr;
    int active_group_index = 0;
    const std::vector<const models::Channel*>* selected_channels = nullptr;
};

/**
 * All actions the view can trigger
 * Presenter handles these
 */
struct MonitoringViewCallbacks
{
    // Toolbar actions
    std::function<void()> on_toggle_monitoring;
    std::function<void()> on_toggle_recording;
    std::function<void()> on_increase_window;
    std::function<void()> on_decrease_window;
    std::function<void()> on_increase_amplitude;
    std::function<void()> on_decrease_amplitude;
    std::function<void()> on_open_impedance_viewer;
    std::function<void()> on_stop_recording;

    // Tab actions
    std::function<void()> on_create_channel_group;
    std::function<void(const std::string&, const ui::TabBounds*)> on_edit_channel_group;
    std::function<void(const models::ChannelsGroup*)> on_group_selected;
};

/**
 * MonitoringView - Completely passive, no logic
 */
class MonitoringView
{
  public:
    MonitoringView() = default;
    ~MonitoringView() = default;

    /**
     * Render the view with given data and callbacks
     * View doesn't know about Model at all!
     */
    void render(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks);

  private:
    elda::ui::TabBar tab_bar_;

    void render_tab_bar(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks);
};

}  // namespace elda::views::monitoring