#ifndef ELDA_MONITORING_VIEW_H
#define ELDA_MONITORING_VIEW_H

#include "UI/chart/chart_data.h"
#include "UI/tabbar/tabbar.h"        // NEW: TabBar component
#include "monitoring_toolbar.h"
#include "monitoring_model.h"
#include "models/channels_group.h"   // NEW: For ChannelsGroup
#include <functional>

namespace elda {

    /**
     * MonitoringView - Pure UI rendering, no business logic
     *
     * Now includes TabBar for channel group selection
     */
    class MonitoringView {
    public:
        MonitoringView() = default;
        ~MonitoringView() = default;

        /**
         * Main render method
         *
         * @param chartData - EEG data for oscilloscope
         * @param toolbarVM - Toolbar display state
         * @param callbacks - User interaction callbacks
         * @param groups - Available channel groups (NEW!)
         * @param activeGroupIndex - Currently active group index (NEW!)
         */
        void render(const ChartData& chartData,
                   const ToolbarViewModel& toolbarVM,
                   const ToolbarCallbacks& callbacks,
                   const std::vector<elda::models::ChannelsGroup>& groups,
                   int activeGroupIndex);

    private:
        elda::ui::TabBar tabBar_;  // NEW: TabBar instance

        /**
         * Render the tab bar for channel group selection
         */
        void renderTabBar(const std::vector<elda::models::ChannelsGroup>& groups,
                         int activeGroupIndex,
                         const ToolbarCallbacks& callbacks);
    };

} // namespace elda

#endif