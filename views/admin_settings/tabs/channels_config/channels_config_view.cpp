#include "channels_config_view.h"

#include <algorithm>
#include <cstdio>

namespace elda::views::channels_config
{

void ChannelsConfigView::render(const std::vector<ChannelConfig>& channels, const ChannelsConfigCallbacks& callbacks)
{
    // Top padding to match Configuration tab
    ImGui::Dummy(ImVec2(0, 8));

    // Left padding to align with tabs (16px indent like tab bar)
    ImGui::Dummy(ImVec2(16, 0));
    ImGui::SameLine();

    // Channel table
    render_table(channels, callbacks);
}

void ChannelsConfigView::render_table(const std::vector<ChannelConfig>& channels,
                                      const ChannelsConfigCallbacks& callbacks)
{
    // Calculate exact table width based on column widths
    constexpr float col_ch = 40.0f;
    constexpr float col_name = 80.0f;
    constexpr float col_type = 80.0f;
    constexpr float col_source = 55.0f;
    constexpr float col_diff = 70.0f;
    constexpr float col_hpf = 90.0f;
    constexpr float col_lpf = 80.0f;
    constexpr float col_adf = 70.0f;
    constexpr float col_enabled = 160.0f;
    constexpr float table_width = col_ch + col_name + col_type + col_source + col_diff + col_hpf + col_lpf + col_adf +
                                  col_enabled + 20.0f;  // +20 for borders

    // Table flags
    ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

    // Calculate available height for table
    float available_height = ImGui::GetContentRegionAvail().y;

    // Use explicit width to prevent stretching
    if (ImGui::BeginTable("##ChannelsTable", 9, flags, ImVec2(table_width, available_height)))
    {
        // Setup columns - all fixed width
        ImGui::TableSetupScrollFreeze(0, 1);  // Freeze header row
        ImGui::TableSetupColumn("Ch", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_ch);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_name);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_type);
        ImGui::TableSetupColumn(
            "Source", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_source);
        ImGui::TableSetupColumn("Diff", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_diff);
        ImGui::TableSetupColumn("HPF", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_hpf);
        ImGui::TableSetupColumn("LPF", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_lpf);
        ImGui::TableSetupColumn("ADF", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_adf);
        ImGui::TableSetupColumn(
            "Enabled", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, col_enabled);
        ImGui::TableHeadersRow();

        // Render rows
        for (const auto& channel : channels)
        {
            render_channel_row(channel, callbacks);
        }

        ImGui::EndTable();
    }
}

void ChannelsConfigView::render_channel_row(const ChannelConfig& channel, const ChannelsConfigCallbacks& callbacks)
{
    ImGui::TableNextRow();
    ImGui::PushID(channel.id);

    // Get row height for vertical centering
    float row_height = ImGui::GetTextLineHeightWithSpacing();

    // Helper to center text horizontally and vertically
    auto center_text = [row_height](const char* text)
    {
        float col_width = ImGui::GetColumnWidth();
        float text_width = ImGui::CalcTextSize(text).x;
        float text_height = ImGui::CalcTextSize(text).y;

        // Horizontal center
        float pad_x = (col_width - text_width) * 0.5f;
        if (pad_x > 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pad_x);

        // Vertical center
        float pad_y = (row_height - text_height) * 0.5f;
        if (pad_y > 0)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + pad_y);

        ImGui::Text("%s", text);
    };

    auto center_widget = [](float widget_width)
    {
        float col_width = ImGui::GetColumnWidth();
        float padding = (col_width - widget_width) * 0.5f;
        if (padding > 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    };

    // Channel ID (centered)
    ImGui::TableNextColumn();
    char id_str[8];
    std::snprintf(id_str, sizeof(id_str), "%d", channel.id);
    center_text(id_str);

    // Name (centered input)
    ImGui::TableNextColumn();
    char name_buf[64];
    std::snprintf(name_buf, sizeof(name_buf), "%s", channel.name.c_str());
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##name", name_buf, sizeof(name_buf)))
    {
        ChannelConfig updated = channel;
        updated.name = name_buf;
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // Signal Type
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1);
    const char* type_items[] = {
        "EEG", "ECG/EMG", "ICG/Imp", "SCG/MOV", "FPG/Resp", "GSR", "Temp", "ROT", "BP", "Force"};
    int type_idx = static_cast<int>(channel.signal_type);
    if (ImGui::Combo("##type", &type_idx, type_items, 10))
    {
        ChannelConfig updated = channel;
        updated.signal_type = static_cast<SignalType>(type_idx);
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // Source Main (centered)
    ImGui::TableNextColumn();
    char src_str[8];
    std::snprintf(src_str, sizeof(src_str), "%d", channel.source_main);
    center_text(src_str);

    // Source Diff
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1);
    const char* diff_items[] = {"Mono", "GND", "REF"};
    int diff_idx = static_cast<int>(channel.source_diff);
    if (ImGui::Combo("##diff", &diff_idx, diff_items, 3))
    {
        ChannelConfig updated = channel;
        updated.source_diff = static_cast<SourceDiff>(diff_idx);
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // HPF
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1);
    const char* hpf_items[] = {"DC", "0.001 Hz", "0.01 Hz", "0.1 Hz", "1 Hz"};
    int hpf_idx = static_cast<int>(channel.hpf);
    if (ImGui::Combo("##hpf", &hpf_idx, hpf_items, 5))
    {
        ChannelConfig updated = channel;
        updated.hpf = static_cast<HPFOption>(hpf_idx);
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // LPF
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1);
    const char* lpf_items[] = {"None", "250 Hz", "500 Hz"};
    int lpf_idx = static_cast<int>(channel.lpf);
    if (ImGui::Combo("##lpf", &lpf_idx, lpf_items, 3))
    {
        ChannelConfig updated = channel;
        updated.lpf = static_cast<LPFOption>(lpf_idx);
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // ADF (Notch)
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1);
    const char* adf_items[] = {"Off", "50 Hz", "60 Hz"};
    int adf_idx = static_cast<int>(channel.adf);
    if (ImGui::Combo("##adf", &adf_idx, adf_items, 3))
    {
        ChannelConfig updated = channel;
        updated.adf = static_cast<ADFOption>(adf_idx);
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }
    ImGui::PopItemWidth();

    // Enabled (centered checkbox)
    ImGui::TableNextColumn();
    center_widget(20.0f);
    bool enabled = channel.enabled;
    if (ImGui::Checkbox("##enabled", &enabled))
    {
        ChannelConfig updated = channel;
        updated.enabled = enabled;
        if (callbacks.on_channel_changed)
        {
            callbacks.on_channel_changed(channel.id, updated);
        }
    }

    ImGui::PopID();
}

}  // namespace elda::views::channels_config