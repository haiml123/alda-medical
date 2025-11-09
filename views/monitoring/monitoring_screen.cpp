#include "monitoring_screen.h"
#include <iostream>

namespace elda {

    MonitoringScreen::MonitoringScreen() {
        std::cout << "[MonitoringScreen] Constructor" << std::endl;
        model_ = std::make_unique<MonitoringModel>();
        view_ = std::make_unique<MonitoringView>();
        presenter_ = std::make_unique<MonitoringPresenter>(*model_, *view_);
    }

    void MonitoringScreen::onEnter() {
        std::cout << "[MonitoringScreen] onEnter" << std::endl;
        presenter_->onEnter();
    }

    void MonitoringScreen::onExit() {
        std::cout << "[MonitoringScreen] onExit" << std::endl;
        presenter_->onExit();
    }

    void MonitoringScreen::update(float deltaTime) {
        presenter_->update(deltaTime);
    }

    void MonitoringScreen::render() {
        presenter_->render();
    }

} // namespace elda