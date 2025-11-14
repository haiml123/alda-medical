#include "monitoring_screen.h"
#include <iostream>

namespace elda::views::monitoring {

    MonitoringScreen::MonitoringScreen(AppState& state, AppStateManager& stateManager, AppRouter& router) {
        std::cout << "[MonitoringScreen] Constructor" << std::endl;

        // Model gets AppState access
        model_ = std::make_unique<MonitoringModel>(state, stateManager);

        // View has no AppState access
        view_ = std::make_unique<MonitoringView>();

        auto channelsPresenter = std::make_unique<channels_selector::ChannelsGroupPresenter>(stateManager);

        channelsPresenter->SetOnGroupsChangedCallback([this]() {
            std::printf("[MonitoringScreen] Groups changed, refreshing available groups\n");
            model_->RefreshAvailableGroups();
        });

        // Presenter orchestrates - needs all three
        presenter_ = std::make_unique<MonitoringPresenter>(*model_, *view_, *channelsPresenter);

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

    void MonitoringScreen::update(float delta_time) {
        presenter_->update(delta_time);
    }

    void MonitoringScreen::render() {
        presenter_->render();
    }

} // namespace elda