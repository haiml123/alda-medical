#ifndef ELDA_MONITORING_TOOLBAR_H
#define ELDA_MONITORING_TOOLBAR_H

#include "imgui.h"
#include "monitoring_model.h"  // For ToolbarViewModel
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
     * Reusable Toolbar Component
     *
     * Renders a horizontal toolbar with monitoring controls, recording buttons,
     * window/amplitude adjustments, and status indicators.
     *
     * @param vm - ToolbarViewModel containing display state
     * @param callbacks - ToolbarCallbacks for user interactions
     * @return Height of the toolbar in pixels
     */
    float MonitoringToolbar(const ToolbarViewModel& vm,
                            const ToolbarCallbacks& callbacks);

} // namespace elda

#endif // ELDA_MONITORING_TOOLBAR_H