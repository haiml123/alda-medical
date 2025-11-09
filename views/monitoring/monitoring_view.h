#ifndef ELDA_MONITORING_VIEW_H
#define ELDA_MONITORING_VIEW_H
#include "UI/chart/chart_data.h"

namespace elda {

    /**
     * MonitoringView - Thin container, just renders the chart
     */
    class MonitoringView {
    public:
        MonitoringView() = default;
        ~MonitoringView() = default;

        void render(const ChartData& chartData);
    };

} // namespace elda

#endif