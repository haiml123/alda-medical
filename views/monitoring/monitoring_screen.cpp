#include "monitoring_screen.h"

#include <iostream>

namespace elda::views::monitoring
{

MonitoringScreen::MonitoringScreen(AppState& state, AppStateManager& state_manager, AppRouter& router)
{
    std::cout << "[MonitoringScreen] Constructor" << std::endl;

    // Model gets AppState access
    model_ = std::make_unique<MonitoringModel>(state, state_manager);

    // View has no AppState access
    view_ = std::make_unique<MonitoringView>();

    auto channels_presenter = std::make_unique<channels_selector::ChannelsGroupPresenter>(state_manager);

    channels_presenter->set_on_groups_changed_callback(
        [this]()
        {
            std::printf("[MonitoringScreen] Groups changed, refreshing available groups\n");
            model_->refresh_available_groups();
        });

    // Presenter orchestrates - needs all three
    presenter_ = std::make_unique<MonitoringPresenter>(*model_, *view_, *channels_presenter);

    // Store channelsPresenter so it doesn't get destroyed
    channels_presenter_ = std::move(channels_presenter);
}

void MonitoringScreen::on_enter()
{
    std::cout << "[MonitoringScreen] onEnter" << std::endl;
    presenter_->on_enter();
}

void MonitoringScreen::on_exit()
{
    std::cout << "[MonitoringScreen] onExit" << std::endl;
    presenter_->on_exit();
}

void MonitoringScreen::update(float delta_time)
{
    presenter_->update(delta_time);
}

void MonitoringScreen::render()
{
    presenter_->render();
}

}  // namespace elda::views::monitoring