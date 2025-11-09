#pragma once

#include "core/router/IScreen.h"
#include "monitoring_model.h"
#include "monitoring_view.h"
#include "monitoring_presenter.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include <memory>

namespace elda {

    class MonitoringScreen : public IScreen {
    public:
        MonitoringScreen(AppState& state, elda::AppStateManager& stateManager);
        ~MonitoringScreen() override = default;

        void onEnter() override;
        void onExit() override;
        void render() override;
        void update(float deltaTime);

    private:
        std::unique_ptr<MonitoringModel> model_;
        std::unique_ptr<MonitoringView> view_;
        std::unique_ptr<MonitoringPresenter> presenter_;
    };

} // namespace elda