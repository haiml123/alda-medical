#include "monitoring_view.h"
#include "imgui.h"
#include "UI/chart/chart.h"

namespace elda {

void MonitoringView::render(const ChartData& chartData,
                           const ToolbarViewModel& toolbarVM,
                           const ToolbarCallbacks& callbacks,
                           const std::vector<elda::models::ChannelsGroup>& groups,
                           int activeGroupIndex) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::Begin("Monitoring", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    // ===== RENDER TOOLBAR AT TOP =====
    MonitoringToolbar(toolbarVM, callbacks);

    // ===== RENDER TAB BAR (NEW!) =====
    renderTabBar(groups, activeGroupIndex, callbacks);

    // ===== RENDER CHART BELOW TOOLBAR AND TABS =====
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("ChartArea", ImVec2(availableSize.x, availableSize.y),
                     false, ImGuiWindowFlags_NoScrollbar);

    DrawChart(chartData);

    ImGui::EndChild();

    ImGui::End();
}

void MonitoringView::renderTabBar(const std::vector<elda::models::ChannelsGroup>& groups,
                                  int activeGroupIndex,
                                  const ToolbarCallbacks& callbacks) {
    // Don't render if no groups available
    if (groups.empty()) {
        return;
    }

    // Convert ChannelsGroup objects to Tab objects for TabBar component
    std::vector<elda::ui::Tab> tabs;
    tabs.reserve(groups.size());

    for (const auto& group : groups) {
        elda::ui::Tab tab;
        tab.label = group.name;
        tab.id = group.name;
        tab.badge = static_cast<int>(group.getSelectedCount());  // Show channel count
        tab.enabled = true;
        tabs.push_back(tab);
    }

    // Update TabBar with current data
    tabBar_.setTabs(tabs);
    tabBar_.setActiveTab(activeGroupIndex);

    // Set the add button callback - button appears automatically!
    tabBar_.setOnAddTab([]() {
        printf("Add button clicked!\n");
        // Your add logic here
    });

    tabBar_.setOnTabDoubleClick([](int index, const elda::ui::Tab& tab) {
        printf("Double-clicked tab %d: %s\n", index, tab.label.c_str());

    });

    // Configure styling (only once)
    static bool styled = false;
    if (!styled) {
        auto& style = tabBar_.getStyle();

        // Match your medical device theme colors
        style.activeColor = ImVec4(0.18f, 0.52f, 0.98f, 1.00f);    // Blue (active)
        style.inactiveColor = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);  // Dark gray (inactive)
        style.hoverColor = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);     // Darker blue (hover)
        style.disabledColor = ImVec4(0.10f, 0.10f, 0.10f, 0.50f);  // Disabled
        style.badgeColor = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);     // Red badge
        style.badgeTextColor = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White text

        // Sizing
        style.height = 40.0f;
        style.buttonPaddingX = 16.0f;
        style.buttonPaddingY = 8.0f;
        style.spacing = 2.0f;
        style.rounding = 4.0f;

        // Features
        style.showSeparator = true;
        style.autoSize = false;
        style.showBadges = true;

        styled = true;
    }

    // Set click callback - when user clicks a tab, apply that group's configuration
    tabBar_.setOnTabClick([&callbacks, &groups](int tabIndex, const elda::ui::Tab& tab) {
        std::printf("[View] Tab clicked: index %d (%s)\n", tabIndex, tab.label.c_str());

        // Apply the selected group's configuration
        if (tabIndex >= 0 && tabIndex < static_cast<int>(groups.size())) {
            if (callbacks.onApplyChannelConfig) {
                callbacks.onApplyChannelConfig(groups[tabIndex]);
            }
        }
    });

    // Render the tab bar
    bool tabChanged = tabBar_.render();

    if (tabChanged) {
        std::printf("[View] Active tab changed to: %d\n", tabBar_.getActiveTab());
    }
}

} // namespace elda