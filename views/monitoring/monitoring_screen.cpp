#include "monitoring_screen.h"
#include <iostream>

namespace elda {

    MonitoringScreen::MonitoringScreen(AppState& state, elda::AppStateManager& stateManager) {
        std::cout << "[MonitoringScreen] Constructor" << std::endl;

        // Model gets AppState access
        model_ = std::make_unique<MonitoringModel>(state, stateManager);

        // View has no AppState access
        view_ = std::make_unique<MonitoringView>();

        auto channelsPresenter = std::make_unique<elda::channels_group::ChannelsGroupPresenter>(stateManager);

        channelsPresenter->SetOnGroupsChangedCallback([this]() {
            std::printf("[MonitoringScreen] Groups changed, refreshing available groups\n");
            model_->RefreshAvailableGroups();
        });

        auto impedancePresenter = std::make_unique<elda::impedance_viewer::ImpedanceViewerScreen>(state, stateManager);


        // Presenter orchestrates - needs all three
        presenter_ = std::make_unique<MonitoringPresenter>(*model_, *view_, *channelsPresenter, *impedancePresenter);

        // Store channelsPresenter so it doesn't get destroyed
        channelsPresenter_ = std::move(channelsPresenter);
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