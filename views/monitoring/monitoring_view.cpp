#include "monitoring_view.h"
#include "monitoring_toolbar.h"
#include "imgui.h"
#include "UI/chart/chart.h"

namespace elda::views::monitoring {

void MonitoringView::render(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::Begin("Monitoring", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    MonitoringToolbar(data, callbacks);

    render_tab_bar(data, callbacks);

    // Render chart
    ImVec2 available_size = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("ChartArea", ImVec2(available_size.x, available_size.y),
                     false, ImGuiWindowFlags_NoScrollbar);

    if (data.chart_data) {
        draw_chart(*data.chart_data, *data.selected_channels);
    }

    ImGui::EndChild();
    ImGui::End();
}

void MonitoringView::render_tab_bar(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks) {
    if (!data.groups || data.groups->empty()) {
        return;
    }

    const auto& groups = *data.groups;

    // Convert to Tab objects
    std::vector<elda::ui::Tab> tabs;
    tabs.reserve(groups.size());

    for (const auto& group : groups) {
        elda::ui::Tab tab;
        tab.label = group.name;
        tab.id = group.id;
        tab.badge = static_cast<int>(group.get_channel_count());
        tab.enabled = true;
        tabs.push_back(tab);
    }

    tab_bar_.set_tabs(tabs);
    tab_bar_.set_active_tab(data.active_group_index);

    // Add button → Create
    tab_bar_.set_on_add_tab([&callbacks]() {
        if (callbacks.on_create_channel_group) {
            callbacks.on_create_channel_group();
        }
    });

    // Single-click → Create
    tab_bar_.set_on_tab_click([&callbacks, &groups](int index, const ui::Tab& tab) {
        if (index >= 0 && index < static_cast<int>(groups.size())) {
            if (callbacks.on_group_selected) {
                callbacks.on_group_selected(&groups[index]);
            }
          }
    });

    // Double-click → Edit
    tab_bar_.set_on_tab_double_click([&callbacks, &groups](int index, const ui::Tab& tab) {
        if (index >= 0 && index < static_cast<int>(groups.size())) {
            if (callbacks.on_edit_channel_group) {
                callbacks.on_edit_channel_group(groups[index].id, tab.bounds);
            }
        }
    });

    // Styling
    static bool styled = false;
    if (!styled) {
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
        styled = true;
    }

    tab_bar_.render();
}

}