#include "user_settings_view.h"

#include <cstring>

namespace elda::views::user_settings
{

void UserSettingsView::setup_form()
{
    // Form fields are added in order, layout handled manually in render_content

    // Mode selection
    form_.add_card_select("mode", "")
        .options({{"Normal", "Standard EEG", 0}, {"MRI", "MR-compatible", 1}, {"Custom", "User-defined", 2}})
        .card_size(140.0f, 80.0f)
        .spacing(16.0f);

    // Patient fields
    form_.add_text("technician", "Technician").width(250.0f);

    form_.add_text("subject_code", "Subject Code").width(250.0f).required().min_length(2, "At least 2 characters");

    form_.add_text("age", "Age").width(80.0f).required().pattern("^[0-9]+$", "Must be a number");

    form_.add_multiline("notes", "Notes").size(250.0f, 80.0f);

    // Custom/NVX fields
    form_.add_select("nvx_mode", "NVX Mode")
        .options({{"Normal Acquisition", 0}, {"Active Shield", 1}, {"Impedance", 2}, {"Test Signal", 3}})
        .width(180.0f);

    form_.add_select("sampling_rate", "Sampling Rate")
        .options({{"250 Hz", 250},
                  {"500 Hz", 500},
                  {"1000 Hz", 1000},
                  {"5000 Hz", 5000},
                  {"10000 Hz", 10000},
                  {"25000 Hz", 25000}})
        .width(180.0f)
        .default_index(2);

    form_.add_select("nvx_gain", "NVX Gain").options({{"0", 0}, {"1", 1}}).width(80.0f);

    form_.add_select("nvx_power_save", "Power Save")
        .options({{"Disable", 0}, {"Enable", 1}})
        .width(120.0f)
        .default_index(1);

    form_.add_select("nvx_scan_freq", "Scan Frequency").options({{"30 Hz", 0}, {"80 Hz", 1}}).width(120.0f);

    form_.add_select("base_adc_sync", "Base ADC Sync")
        .options({{"Internal", 0}, {"External", 1}, {"Manual", 2}})
        .width(120.0f);

    form_.add_select("sw_impedance_reduction", "SW Imp. Reduction")
        .options({{"Off", 0}, {"On", 1}})
        .width(120.0f)
        .default_index(1);

    // No layout - we handle it manually in render_content
}

void UserSettingsView::render(const UserSettingsViewData& data, const UserSettingsViewCallbacks& callbacks)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##UserSettingsScreen",
                 nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header using reusable component - back button hidden
    elda::ui::render_screen_header({.title = "Session Settings",
                                    .show_back_button = false,
                                    .on_back = callbacks.on_back,
                                    .buttons = {
                                        {.label = "ADMIN",
                                         .on_click = callbacks.on_admin,
                                         .enabled = data.can_proceed,
                                         .primary = true,
                                         .width = 140.0f},
                                        {.label = "SAVE",
                                         .on_click = callbacks.on_proceed,
                                         .enabled = data.can_proceed,
                                         .primary = true,
                                         .width = 140.0f},
                                    }});

    // Content
    render_content();

    ImGui::End();
    ImGui::PopStyleVar();
}

void UserSettingsView::render_content()
{
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32, 24));
    ImGui::BeginChild("##Content", content_size, false);

    // Top padding
    ImGui::Dummy(ImVec2(0, 8));

    // Center the form with max width
    const float max_form_width = 950.0f;
    float available_width = ImGui::GetContentRegionAvail().x;
    float padding = (available_width > max_form_width) ? (available_width - max_form_width) / 2.0f : 0.0f;

    if (padding > 0)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    }

    ImGui::BeginGroup();

    const float label_width = 130.0f;
    bool is_custom = form_.get_selected_index("mode") == 2;

    // Section: Acquisition Mode
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("Acquisition Mode");
    ImGui::PopStyleColor();

    if (auto* field = form_.get_field("mode"))
    {
        field->render(label_width);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Two columns: Patient Info | Hardware Config (if custom)
    float col_width = is_custom ? max_form_width / 2.0f : max_form_width;
    ImGui::Columns(is_custom ? 2 : 1, nullptr, false);
    if (is_custom)
    {
        ImGui::SetColumnWidth(0, col_width);
        ImGui::SetColumnWidth(1, col_width);
    }

    // Left column: Patient Information
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("Patient Information");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if (auto* f = form_.get_field("technician"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("subject_code"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("age"))
        f->render(label_width);
    ImGui::Spacing();
    if (auto* f = form_.get_field("notes"))
        f->render(label_width);

    // Right column: Hardware Configuration (only if Custom mode)
    if (is_custom)
    {
        ImGui::NextColumn();

        const float hw_label_width = 145.0f;  // Wider for longer labels

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
        ImGui::Text("Hardware Configuration");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (auto* f = form_.get_field("nvx_mode"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("sampling_rate"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("nvx_gain"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("nvx_power_save"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("nvx_scan_freq"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("base_adc_sync"))
            f->render(hw_label_width);
        ImGui::Spacing();
        if (auto* f = form_.get_field("sw_impedance_reduction"))
            f->render(hw_label_width);
    }

    ImGui::Columns(1);

    ImGui::EndGroup();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

}  // namespace elda::views::user_settings