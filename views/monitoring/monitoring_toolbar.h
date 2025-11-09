#ifndef ELDA_MONITORING_TOOLBAR_H
#define ELDA_MONITORING_TOOLBAR_H

#include "imgui.h"
#include "monitoring_view.h"  // For MonitoringViewData and MonitoringViewCallbacks
#include <functional>

namespace elda {

    /**
     * Reusable Toolbar Component
     *
     * Renders a horizontal toolbar with monitoring controls, recording buttons,
     * window/amplitude adjustments, and status indicators.
     *
     * @param data - MonitoringViewData containing display state
     * @param callbacks - MonitoringViewCallbacks for user interactions
     * @return Height of the toolbar in pixels
     */
    float MonitoringToolbar(const MonitoringViewData& data,
                            const MonitoringViewCallbacks& callbacks);

} // namespace elda

#endif // ELDA_MONITORING_TOOLBAR_H