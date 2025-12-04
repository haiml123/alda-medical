#include "admin_settings_presenter.h"

#include <iostream>

namespace elda::views::admin_settings
{

AdminSettingsPresenter::AdminSettingsPresenter(AdminSettingsModel& model,
                                               AdminSettingsView& view,
                                               AppRouter& router,
                                               AppStateManager& state_manager)
    : model_(model), view_(view), router_(router), state_manager_(state_manager)
{
    view_.setup_form();
    setup_callbacks();
}

void AdminSettingsPresenter::on_enter()
{
    std::cout << "[AdminSettings] Enter\n";
}

void AdminSettingsPresenter::on_exit()
{
    std::cout << "[AdminSettings] Exit\n";
}

void AdminSettingsPresenter::update(float /*delta_time*/)
{
    // Form manages its own state
}

void AdminSettingsPresenter::render()
{
    AdminSettingsViewData view_data;
    view_data.active_tab = model_.active_tab();

    view_.render(view_data, callbacks_);
}

void AdminSettingsPresenter::setup_callbacks()
{
    callbacks_.on_save = [this]()
    {
        handle_save();
    };

    callbacks_.on_close = [this]()
    {
        handle_close();
    };

    callbacks_.on_tab_changed = [this](int tab_index)
    {
        model_.set_active_tab(tab_index);
        std::cout << "[AdminSettings] Tab changed to: " << tab_index << "\n";
    };
}

void AdminSettingsPresenter::sync_form_to_model()
{
    auto& form = view_.form();

    // Device config
    auto& device = model_.device_config();
    device.nvx_mode = form.get_selected_value("nvx_mode");
    device.nvx_rate = form.get_selected_value("nvx_rate");
    device.nvx_decimation = form.get_selected_value("nvx_decimation");
    device.nvx_gain = form.get_selected_value("nvx_gain");
    device.nvx_power_save = form.get_selected_value("nvx_power_save");
    device.nvx_scan_freq = form.get_selected_value("nvx_scan_freq");

    // Output config
    auto& output = model_.output_config();
    output.output_file_type = form.get_selected_value("output_file_type");
    output.base_adc_sync = form.get_selected_value("base_adc_sync");
    output.sw_impedance_reduction = form.get_selected_value("sw_impedance_reduction");

    // Channel defaults
    auto& channel = model_.channel_defaults();
    channel.source = form.get_selected_value("channel_source");
    channel.hpf = form.get_selected_value("channel_hpf");
    channel.lpf = form.get_selected_value("channel_lpf");
    channel.adf = form.get_selected_value("channel_adf");
}

void AdminSettingsPresenter::handle_save()
{
    sync_form_to_model();

    // Log settings
    const auto& device = model_.device_config();
    const auto& output = model_.output_config();
    const auto& channel = model_.channel_defaults();

    std::cout << "[AdminSettings] Saving settings:\n"
              << "  Device:\n"
              << "    NVX Mode: " << device.nvx_mode << "\n"
              << "    NVX Rate: " << device.nvx_rate << "\n"
              << "    NVX Decimation: " << device.nvx_decimation << "\n"
              << "    NVX Gain: " << device.nvx_gain << "\n"
              << "    Power Save: " << (device.nvx_power_save ? "Enable" : "Disable") << "\n"
              << "    Scan Freq: " << (device.nvx_scan_freq ? "80Hz" : "30Hz") << "\n"
              << "  Output:\n"
              << "    File Type: " << output.output_file_type << "\n"
              << "    ADC Sync: " << output.base_adc_sync << "\n"
              << "    SW Imp. Red: " << (output.sw_impedance_reduction ? "On" : "Off") << "\n"
              << "  Channel Defaults:\n"
              << "    Source: " << channel.source << "\n"
              << "    HPF: " << channel.hpf << "\n"
              << "    LPF: " << channel.lpf << "\n"
              << "    ADF: " << channel.adf << "\n";

    // TODO: Save to persistent storage / AppStateManager

    // Return to previous screen
    router_.return_to_previous_mode();
}

void AdminSettingsPresenter::handle_close()
{
    std::cout << "[AdminSettings] Close without saving\n";
    router_.return_to_previous_mode();
}

}  // namespace elda::views::admin_settings
