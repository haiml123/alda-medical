#ifndef ELDA_MONITORING_VIEW_H
#define ELDA_MONITORING_VIEW_H

#include "UI/chart/chart_data.h"
#include "monitoring_toolbar.h"  // Now includes ToolbarCallbacks definition
#include "monitoring_model.h"
#include <functional>

namespace elda {

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
    };

} // namespace elda

#endif