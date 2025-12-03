#include "user_settings_presenter.h"
#include <iostream>

namespace elda::views::user_settings {

UserSettingsPresenter::UserSettingsPresenter(UserSettingsModel& model,
                                             UserSettingsView& view,
                                             AppRouter& router,
                                             AppStateManager& state_manager)
    : model_(model)
    , view_(view)
    , router_(router)
    , state_manager_(state_manager)
{
    view_.setup_form();
    setup_callbacks();
}

void UserSettingsPresenter::on_enter() {
    std::cout << "[UserSettings] Enter\n";
}

void UserSettingsPresenter::on_exit() {
    std::cout << "[UserSettings] Exit\n";
}

void UserSettingsPresenter::update(float /*delta_time*/) {
    // Form manages its own state
}

void UserSettingsPresenter::render() {
    auto& form = view_.form();
    form.validate();

    UserSettingsViewData view_data;
    view_data.can_proceed = form.is_valid();

    view_.render(view_data, callbacks_);
}

void UserSettingsPresenter::setup_callbacks() {
    callbacks_.on_proceed = [this]() {
        handle_proceed();
    };

    callbacks_.on_back = [this]() {
        std::cout << "[UserSettings] Back\n";
        router_.return_to_previous_mode();
    };
    callbacks_.on_admin = [this]() {
        std::cout << "[UserSettings] Admin\n";
        router_.transition_to(AppMode::ADMIN_SETTINGS);
    };
}

void UserSettingsPresenter::sync_form_to_model() {
    auto& form = view_.form();

    // Mode
    model_.set_mode(static_cast<AcquisitionMode>(form.get_selected_index("mode")));

    // Patient info
    auto& patient = model_.patient();
    patient.technician = form.get_string("technician");
    patient.subject_code = form.get_string("subject_code");
    patient.age = form.get_string("age");
    patient.notes = form.get_string("notes");

    // MRI settings
    if (model_.get_mode() == AcquisitionMode::MRI) {
        auto& mri = model_.mri_settings();
        mri.scanner_id = form.get_string("scanner_id");
        mri.tr_ms = form.get_int("tr_ms");
        mri.gradient_correction = form.get_bool("gradient_correction");
    }

    // Custom/NVX settings
    if (model_.get_mode() == AcquisitionMode::CUSTOM) {
        auto& custom = model_.custom_settings();
        custom.nvx_mode = form.get_selected_value("nvx_mode");
        custom.sampling_rate = form.get_selected_value("sampling_rate");
        custom.nvx_gain = form.get_selected_value("nvx_gain");
        custom.nvx_power_save = form.get_selected_value("nvx_power_save");
        custom.nvx_scan_freq = form.get_selected_value("nvx_scan_freq");
        custom.base_adc_sync = form.get_selected_value("base_adc_sync");
        custom.sw_impedance_reduction = form.get_selected_value("sw_impedance_reduction");
    }
}

void UserSettingsPresenter::handle_proceed() {
    auto& form = view_.form();

    // Mark all fields dirty so validation errors show
    form.mark_all_dirty();

    if (!form.validate()) {
        std::cout << "[UserSettings] Validation failed: " << form.get_first_error() << "\n";
        return;
    }

    sync_form_to_model();

    // Log
    const auto& patient = model_.patient();
    std::cout << "[UserSettings] Proceeding:\n"
              << "  Mode: " << acquisition_mode_to_string(model_.get_mode()) << "\n"
              << "  Technician: " << patient.technician << "\n"
              << "  Subject: " << patient.subject_code << "\n"
              << "  Age: " << patient.age << "\n";

    if (model_.get_mode() == AcquisitionMode::CUSTOM) {
        const auto& custom = model_.custom_settings();
        std::cout << "  NVX Mode: " << custom.nvx_mode << "\n"
                  << "  Sampling Rate: " << custom.sampling_rate << " Hz\n"
                  << "  NVX Gain: " << custom.nvx_gain << "\n"
                  << "  Power Save: " << (custom.nvx_power_save ? "Enable" : "Disable") << "\n"
                  << "  Scan Freq: " << (custom.nvx_scan_freq ? "80Hz" : "30Hz") << "\n"
                  << "  Base ADC Sync: " << custom.base_adc_sync << "\n"
                  << "  SW Impedance Reduction: " << (custom.sw_impedance_reduction ? "On" : "Off") << "\n";
    }

    router_.transition_to(AppMode::CAP_PLACEMENT);
}

} // namespace elda::views::user_settings