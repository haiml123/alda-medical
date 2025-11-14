#pragma once

#include "core/router/IScreen.h"
#include "monitoring_model.h"
#include "monitoring_view.h"
#include "monitoring_presenter.h"
#include "views/channels_selector_modal/channels_group_presenter.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include <memory>

#include "core/router/app_router.h"

namespace elda::views::monitoring {

    class MonitoringScreen : public IScreen {
    public:
        MonitoringScreen(AppState& state, AppStateManager& stateManager, AppRouter& router);
        ~MonitoringScreen() override = default;

        void onEnter() override;
        void onExit() override;
        void render() override;
        void update(float dt) override;

    private:
        std::unique_ptr<MonitoringModel> model_;
        std::unique_ptr<MonitoringView> view_;
        std::unique_ptr<channels_selector::ChannelsGroupPresenter> channelsPresenter_;
        std::unique_ptr<MonitoringPresenter> presenter_;
    };

} // namespace elda