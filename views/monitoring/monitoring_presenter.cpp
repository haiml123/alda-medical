#include "monitoring_presenter.h"
#include <iostream>

namespace elda {

    MonitoringPresenter::MonitoringPresenter(MonitoringModel& model, MonitoringView& view)
        : model_(model), view_(view) {
    }

    void MonitoringPresenter::onEnter() {
        std::cout << "[Presenter] Enter monitoring" << std::endl;
        model_.startAcquisition();
    }

    void MonitoringPresenter::onExit() {
        std::cout << "[Presenter] Exit monitoring" << std::endl;
        model_.stopAcquisition();
    }

    void MonitoringPresenter::update(float deltaTime) {
        model_.update(deltaTime);
    }

    void MonitoringPresenter::render() {
        // Get clean chart data from model
        ChartData chartData = model_.getChartData();

        // Pass to view for rendering
        view_.render(chartData);
    }

} // namespace elda