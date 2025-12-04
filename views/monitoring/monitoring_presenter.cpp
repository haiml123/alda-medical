#include "monitoring_presenter.h"

#include "monitoring_model.h"
#include "monitoring_view.h"

#include <iostream>

namespace elda::views::monitoring
{

MonitoringPresenter::MonitoringPresenter(MonitoringModel& model,
                                         MonitoringView& view,
                                         channels_selector::ChannelsGroupPresenter& channels_presenter)
    : model_(model),
      view_(view),
      channels_presenter_(&channels_presenter),
      channel_modal_position_(0, 0),
      use_custom_modal_position_(false)
{
    setup_callbacks();
}

void MonitoringPresenter::on_enter()
{
    std::cout << "[Presenter] Enter monitoring" << std::endl;

    model_.start_acquisition();

    model_.add_state_observer(
        [this](StateField field)
        {
            std::cout << "[Presenter] State changed: " << static_cast<int>(field) << std::endl;
        });
}

void MonitoringPresenter::on_exit()
{
    std::cout << "[Presenter] Exit monitoring" << std::endl;
    model_.stop_acquisition();
}

void MonitoringPresenter::update(float delta_time)
{
    model_.update(delta_time);
}

void MonitoringPresenter::render()
{
    MonitoringViewData view_data;

    view_data.chart_data = &model_.get_chart_data();
    view_data.groups = &model_.get_available_groups();

    view_data.monitoring = model_.is_monitoring();
    view_data.can_record = model_.can_record();
    view_data.recording_active = model_.is_recording_active();
    view_data.recording_state = model_.get_recording_state();
    view_data.currently_recording = model_.is_currently_recording();
    view_data.currently_paused = model_.is_currently_paused();
    view_data.window_seconds = model_.get_window_seconds();
    view_data.amplitude_micro_volts = model_.get_amplitude_micro_volts();
    view_data.sample_rate_hz = model_.get_sample_rate_hz();
    view_data.active_group_index = model_.get_active_group_index();
    view_data.selected_channels = &model_.get_selected_channels();

    view_.render(view_data, callbacks_);

    if (channels_presenter_->is_open())
    {
        ImVec2 modal_size = ImVec2(300, 550);
        ImVec2 modal_pos =
            use_custom_modal_position_ ? channel_modal_position_ : calculate_default_modal_position(modal_size);

        channels_presenter_->render(modal_pos);
    }
}

ImVec2 MonitoringPresenter::calculate_default_modal_position(ImVec2 modal_size) const
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    return ImVec2(viewport->Pos.x + (viewport->Size.x - modal_size.x) * 0.5f, viewport->Pos.y + 100.0f);
}

void MonitoringPresenter::setup_callbacks()
{
    callbacks_.on_toggle_monitoring = [this]()
    {
        model_.toggle_monitoring();
    };
    callbacks_.on_toggle_recording = [this]()
    {
        model_.toggle_recording();
    };
    callbacks_.on_stop_recording = [this]()
    {
        model_.stop_recording();
    };
    callbacks_.on_increase_window = [this]()
    {
        model_.increase_window();
    };
    callbacks_.on_decrease_window = [this]()
    {
        model_.decrease_window();
    };
    callbacks_.on_increase_amplitude = [this]()
    {
        model_.increase_amplitude();
    };
    callbacks_.on_decrease_amplitude = [this]()
    {
        model_.decrease_amplitude();
    };

    callbacks_.on_create_channel_group = [this]()
    {
        channels_presenter_->open("",
                                  [this](const models::ChannelsGroup& new_group)
                                  {
                                      model_.apply_channel_configuration(new_group);
                                      model_.on_group_selected(new_group);
                                  });
    };

    callbacks_.on_open_impedance_viewer = [this]()
    {
        std::cout << "Open impedance viewer - navigate to impedance screen" << std::endl;
        // Navigation handled by screen manager
    };

    callbacks_.on_group_selected = [this](const models::ChannelsGroup* group)
    {
        if (!group)
            return;
        model_.on_group_selected(*group);
    };

    callbacks_.on_edit_channel_group = [this](const std::string& id, const ui::TabBounds* bounds)
    {
        if (bounds)
        {
            use_custom_modal_position_ = true;
            channel_modal_position_.x = bounds->x;
            channel_modal_position_.y = bounds->y + bounds->height;
        }
        else
        {
            use_custom_modal_position_ = false;
        }

        channels_presenter_->open(id,
                                  [this](const elda::models::ChannelsGroup& edited_group)
                                  {
                                      model_.apply_channel_configuration(edited_group);
                                      model_.on_group_selected(edited_group);
                                  });
    };
}

}  // namespace elda::views::monitoring