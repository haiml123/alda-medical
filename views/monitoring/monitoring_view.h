#ifndef ELDA_MONITORING_VIEW_H
#define ELDA_MONITORING_VIEW_H

#include "UI/chart/chart_data.h"
#include "monitoring_model.h"
#include "models/channels_group.h"
#include <functional>

namespace elda {

    /**
     * Toolbar callbacks - View tells Presenter what user did
     */
    struct ToolbarCallbacks {
        std::function<void()> onToggleMonitoring;
        std::function<void()> onToggleRecording;
        std::function<void()> onIncreaseWindow;
        std::function<void()> onDecreaseWindow;
        std::function<void()> onIncreaseAmplitude;
        std::function<void()> onDecreaseAmplitude;
        std::function<void(const elda::models::ChannelsGroup&)> onApplyChannelConfig;
    };

    /**
     * MonitoringView - Pure UI rendering, no business logic
     */
    class MonitoringView {
    public:
        MonitoringView() = default;
        ~MonitoringView() = default;

        void render(const ChartData& chartData,
                   const ToolbarViewModel& toolbarVM,
                   const ToolbarCallbacks& callbacks);

    private:
        float renderToolbar(const ToolbarViewModel& vm, const ToolbarCallbacks& callbacks);
    };

} // namespace elda

#endif