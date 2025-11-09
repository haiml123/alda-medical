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
        // Get data from model
        const ChartData& chartData = model_.getChartData();
        ToolbarViewModel toolbarVM = model_.getToolbarViewModel();

        // Setup callbacks
        ToolbarCallbacks callbacks;

        callbacks.onToggleMonitoring = [this]() {
            model_.toggleMonitoring();
        };

        callbacks.onToggleRecording = [this]() {
            model_.toggleRecording();
        };

        callbacks.onIncreaseWindow = [this]() {
            model_.increaseWindow();
        };

        callbacks.onDecreaseWindow = [this]() {
            model_.decreaseWindow();
        };

        callbacks.onIncreaseAmplitude = [this]() {
            model_.increaseAmplitude();
        };

        callbacks.onDecreaseAmplitude = [this]() {
            model_.decreaseAmplitude();
        };

        callbacks.onApplyChannelConfig = [this](const elda::models::ChannelsGroup& group) {
            model_.applyChannelConfiguration(group);
        };

        // Pass to view for rendering
        view_.render(chartData, toolbarVM, callbacks);
    }

} // namespace elda