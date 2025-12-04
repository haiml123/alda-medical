#include "admin_settings_view.h"

#include <cstring>
#include <memory>

namespace elda::views::admin_settings
{

AdminSettingsView::AdminSettingsView()
{
    setup_tabs();

    // Initialize channels config presenter
    channels_presenter_ = std::make_unique<channels_config::ChannelsConfigPresenter>(channels_model_, channels_view_);
}

void AdminSettingsView::setup_tabs()
{
    std::vector<elda::ui::Tab> tabs;

    elda::ui::Tab config_tab;
    config_tab.label = "Configuration";
    config_tab.id = "config";
    config_tab.badge = -1;
    config_tab.enabled = true;
    tabs.push_back(config_tab);

    elda::ui::Tab electrode_tab;
    electrode_tab.label = "Electrode Config";
    electrode_tab.id = "electrodes";
    electrode_tab.badge = -1;
    electrode_tab.enabled = true;
    tabs.push_back(electrode_tab);

    tab_bar_.set_tabs(tabs);

    // Style the tabs to match monitoring screen
    auto& style = tab_bar_.get_style();
    style.active_color = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
    style.inactive_color = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    style.hover_color = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    style.height = 40.0f;
    style.button_padding_x = 16.0f;
    style.button_padding_y = 8.0f;
    style.spacing = 2.0f;
    style.rounding = 4.0f;
    style.show_separator = true;
    style.auto_size = false;
    style.show_badges = true;
}

void AdminSettingsView::setup_form()
{
    // ========================================================================
    // NVX Device Settings (per customer requirements)
    // ========================================================================

    form_.add_select("nvx_mode", "NVX Mode")
        .options({{"Normal Acquisition", 0}, {"Active Shield", 1}, {"Impedance", 2}, {"Test Signal", 3}})
        .width(180.0f);

    form_.add_select("nvx_rate", "NVX Rate").options({{"10 kHz", 0}, {"50 kHz", 1}, {"100 kHz", 2}}).width(120.0f);

    form_.add_select("nvx_decimation", "NVX Decimation")
        .options({{"0", 0}, {"2", 2}, {"5", 5}, {"10", 10}, {"20", 20}, {"40", 40}})
        .width(80.0f);

    form_.add_select("nvx_gain", "NVX Gain").options({{"0", 0}, {"1", 1}}).width(80.0f);

    form_.add_select("nvx_power_save", "NVX Power Save")
        .options({{"Disable", 0}, {"Enable", 1}})
        .width(120.0f)
        .default_index(1);

    form_.add_select("nvx_scan_freq", "NVX Scan Freq").options({{"30 Hz", 0}, {"80 Hz", 1}}).width(100.0f);

    // ========================================================================
    // Output Settings
    // ========================================================================

    form_.add_select("output_file_type", "Output File Type")
        .options({{"EDF+", 0}, {"EEG", 1}, {"Raw", 2}})
        .width(100.0f);

    form_.add_select("base_adc_sync", "Base ADC Sync")
        .options({{"Internal", 0}, {"External", 1}, {"Manual", 2}})
        .width(120.0f);

    form_.add_select("sw_impedance_reduction", "SW Imp. Reduction")
        .options({{"Off", 0}, {"On", 1}})
        .width(100.0f)
        .default_index(1);

    // ========================================================================
    // Channel Settings (defaults for all channels)
    // ========================================================================

    form_.add_select("channel_source", "Source")
        .options({{"Diff", 0}, {"GND", 1}, {"REF", 2}})
        .width(100.0f)
        .default_index(2);  // REF default

    form_.add_select("channel_hpf", "HPF")
        .options({{"DC", 0}, {"0.001 Hz", 1}, {"0.01 Hz", 2}, {"0.1 Hz", 3}, {"1 Hz", 4}})
        .width(120.0f)
        .default_index(1);  // 0.001 Hz default

    form_.add_select("channel_lpf", "LPF")
        .options({{"None", 0}, {"250 Hz", 1}, {"500 Hz", 2}})
        .width(100.0f)
        .default_index(1);  // 250 Hz default

    form_.add_select("channel_adf", "ADF (Notch)")
        .options({{"None", 0}, {"50 Hz", 1}, {"60 Hz", 2}})
        .width(100.0f);  // None default
}

void AdminSettingsView::render(const AdminSettingsViewData& data, const AdminSettingsViewCallbacks& callbacks)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##AdminSettingsScreen",
                 nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header
    elda::ui::render_screen_header(
        {.title = "Admin Settings",
         .show_back_button = true,
         .on_back = callbacks.on_close,
         .buttons = {
             {.label = "SAVE", .on_click = callbacks.on_save, .enabled = true, .primary = true, .width = 100.0f}}});

    // Tab bar
    render_tab_bar(data, callbacks);

    // Content based on active tab
    render_content(data);

    ImGui::End();
    ImGui::PopStyleVar();
}

void AdminSettingsView::render_tab_bar(const AdminSettingsViewData& data, const AdminSettingsViewCallbacks& callbacks)
{
    tab_bar_.set_active_tab(data.active_tab);

    tab_bar_.set_on_tab_click(
        [&callbacks](int index, const elda::ui::Tab&)
        {
            if (callbacks.on_tab_changed)
            {
                callbacks.on_tab_changed(index);
            }
        });

    // Add padding around tab bar like monitoring screen
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
    ImGui::Dummy(ImVec2(0, 8));  // Top padding
    ImGui::Indent(16.0f);        // Left padding
    tab_bar_.render();
    ImGui::Unindent(16.0f);
    ImGui::Dummy(ImVec2(0, 8));  // Bottom padding
    ImGui::PopStyleColor();
}

void AdminSettingsView::render_content(const AdminSettingsViewData& data)
{
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32, 24));
    ImGui::BeginChild("##Content", content_size, false);

    if (data.active_tab == 0)
    {
        render_configuration_tab();
    }
    else
    {
        render_electrode_config_tab();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void AdminSettingsView::render_configuration_tab()
{
    // Top padding
    ImGui::Dummy(ImVec2(0, 8));

    // Center the form with max width
    const float max_form_width = 900.0f;
    float available_width = ImGui::GetContentRegionAvail().x;
    float padding = (available_width > max_form_width) ? (available_width - max_form_width) / 2.0f : 0.0f;

    if (padding > 0)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    }

    ImGui::BeginGroup();

    const float label_width = 145.0f;
    const float col_width = max_form_width / 2.0f;

    // Two columns layout
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, col_width);
    ImGui::SetColumnWidth(1, col_width);

    // ========================================================================
    // Left column: Device Configuration
    // ========================================================================
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("Device Configuration");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    if (auto* f = form_.get_field("nvx_mode"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("nvx_rate"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("nvx_decimation"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("nvx_gain"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("nvx_power_save"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("nvx_scan_freq"))
        f->render(label_width);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Output Settings section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("Output Settings");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    if (auto* f = form_.get_field("output_file_type"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("base_adc_sync"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("sw_impedance_reduction"))
        f->render(label_width);

    // ========================================================================
    // Right column: Channel Settings (defaults)
    // ========================================================================
    ImGui::NextColumn();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("Channel Settings (Defaults)");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    if (auto* f = form_.get_field("channel_source"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("channel_hpf"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("channel_lpf"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("channel_adf"))
        f->render(label_width);

    ImGui::Columns(1);

    ImGui::EndGroup();
}

void AdminSettingsView::render_electrode_config_tab()
{
    // Render the embedded channels configuration
    channels_presenter_->render();
}

}  // namespace elda::views::admin_settings
