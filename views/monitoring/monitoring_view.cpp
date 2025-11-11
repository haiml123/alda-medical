#include "monitoring_view.h"
#include "monitoring_toolbar.h"
#include "imgui.h"
#include "UI/chart/chart.h"

namespace elda {

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

    renderTabBar(data, callbacks);

    // Render chart
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("ChartArea", ImVec2(availableSize.x, availableSize.y),
                     false, ImGuiWindowFlags_NoScrollbar);

    if (data.chartData) {
        DrawChart(*data.chartData, *data.selectedChannels);
    }

    ImGui::EndChild();
    ImGui::End();
}

void MonitoringView::renderTabBar(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks) {
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
        tab.badge = static_cast<int>(group.getChannelCount());
        tab.enabled = true;
        tabs.push_back(tab);
    }

    tabBar_.setTabs(tabs);
    tabBar_.setActiveTab(data.activeGroupIndex);

    // Add button → Create
    tabBar_.setOnAddTab([&callbacks]() {
        if (callbacks.onCreateChannelGroup) {
            callbacks.onCreateChannelGroup();
        }
    });

    // Single-click → Create
    tabBar_.setOnTabClick([&callbacks, &groups](int index, const ui::Tab& tab) {
        if (index >= 0 && index < static_cast<int>(groups.size())) {
            if (callbacks.onGroupSelected) {
                callbacks.onGroupSelected(&groups[index]);
            }
          }
    });

    // Double-click → Edit
    tabBar_.setOnTabDoubleClick([&callbacks, &groups](int index, const ui::Tab& tab) {
        if (index >= 0 && index < static_cast<int>(groups.size())) {
            if (callbacks.onEditChannelGroup) {
                callbacks.onEditChannelGroup(groups[index].id, tab.bounds);
            }
        }
    });

    // Styling
    static bool styled = false;
    if (!styled) {
        auto& style = tabBar_.getStyle();
        style.activeColor = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);
        style.inactiveColor = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
        style.hoverColor = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
        style.height = 40.0f;
        style.buttonPaddingX = 16.0f;
        style.buttonPaddingY = 8.0f;
        style.spacing = 2.0f;
        style.rounding = 4.0f;
        style.showSeparator = true;
        style.autoSize = false;
        style.showBadges = true;
        styled = true;
    }

    tabBar_.render();
}

} // namespace elda