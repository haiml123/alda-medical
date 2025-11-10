#ifndef ELDA_MONITORING_VIEW_H
#define ELDA_MONITORING_VIEW_H

#include "UI/chart/chart_data.h"
#include "UI/tabbar/tabbar.h"
#include "models/channels_group.h"
#include <functional>
#include <vector>

namespace elda {

    /**
     * All data the view needs to display
     * Presenter collects this from Model
     */
    struct MonitoringViewData {
        // Chart
        const ChartData* chartData = nullptr;

        // Toolbar state
        bool monitoring = false;
        bool canRecord = false;
        bool recordingActive = false;
        bool currentlyPaused = false;
        int windowSeconds = 10;
        int amplitudeMicroVolts = 100;
        double sampleRateHz = 1000.0;

        // Tab bar
        const std::vector<elda::models::ChannelsGroup>* groups = nullptr;
        int activeGroupIndex = 0;
    };

    /**
     * All actions the view can trigger
     * Presenter handles these
     */
    struct MonitoringViewCallbacks {
        // Toolbar actions
        std::function<void()> onToggleMonitoring;
        std::function<void()> onToggleRecording;
        std::function<void()> onIncreaseWindow;
        std::function<void()> onDecreaseWindow;
        std::function<void()> onIncreaseAmplitude;
        std::function<void()> onDecreaseAmplitude;

        // Tab actions
        std::function<void()> onCreateChannelGroup;
        std::function<void(const std::string&, const ui::TabBounds*)> onEditChannelGroup;
    };

    /**
     * MonitoringView - Completely passive, no logic
     */
    class MonitoringView {
    public:
        MonitoringView() = default;
        ~MonitoringView() = default;

        /**
         * Render the view with given data and callbacks
         * View doesn't know about Model at all!
         */
        void render(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks);

    private:
        elda::ui::TabBar tabBar_;

        void renderTabBar(const MonitoringViewData& data, const MonitoringViewCallbacks& callbacks);
    };

} // namespace elda

#endif // ELDA_MONITORING_VIEW_H