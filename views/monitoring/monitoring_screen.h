#pragma once

#include "core/router/IScreen.h"
#include "monitoring_model.h"
#include "monitoring_view.h"
#include "monitoring_presenter.h"
#include <memory>

namespace elda {

    class MonitoringScreen : public IScreen {
    public:
        MonitoringScreen();
        ~MonitoringScreen() override = default;

        // IScreen interface
        void onEnter() override;
        void onExit() override;
        void render() override;

        // Additional methods
        void update(float deltaTime);

    private:
        std::unique_ptr<MonitoringModel> model_;
        std::unique_ptr<MonitoringView> view_;
        std::unique_ptr<MonitoringPresenter> presenter_;
        // name_ removed - not needed!
    };

} // namespace elda