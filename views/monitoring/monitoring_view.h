#pragma once
#include "UI/chart/chart_data.h"
#include "UI/tabbar/tabbar.h"
#include "models/channels_group.h"
#include <functional>
#include <vector>

#include "core/core.h"
#include "models/channel.h"


namespace elda::views::monitoring {

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
        bool currentlyRecording = false;
        bool currentlyPaused = false;
        int windowSeconds = 10;
        int amplitudeMicroVolts = 100;
        double sampleRateHz = 1000.0;
        RecordingState recordingState;

        // Tab bar
        const std::vector<elda::models::ChannelsGroup>* groups = nullptr;
        int activeGroupIndex = 0;
        const std::vector<const models::Channel*>* selectedChannels = nullptr;
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
        std::function<void()> onOpenImpedanceViewer;
        std::function<void()> onStopRecording;

        // Tab actions
        std::function<void()> onCreateChannelGroup;
        std::function<void(const std::string&, const ui::TabBounds*)> onEditChannelGroup;
        std::function<void(const models::ChannelsGroup*)> onGroupSelected;
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

}