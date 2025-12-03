#pragma once

#include "admin_settings_model.h"
#include "admin_settings_view.h"
#include "admin_settings_presenter.h"
#include "core/router/IScreen.h"
#include "core/router/app_router.h"
#include "core/app_state_manager.h"

namespace elda::views::admin_settings {

class AdminSettingsScreen : public IScreen {
public:
    AdminSettingsScreen(AppState& app_state,
                        AppStateManager& state_manager,
                        AppRouter& router)
        : app_state_(app_state)
        , presenter_(model_, view_, router, state_manager)
    {}

    void on_enter() override {
        presenter_.on_enter();
    }

    void on_exit() override {
        presenter_.on_exit();
    }

    void update(float delta_time) override {
        presenter_.update(delta_time);
    }

    void render() override {
        presenter_.render();
    }

private:
    AppState& app_state_;
    AdminSettingsModel model_;
    AdminSettingsView view_;
    AdminSettingsPresenter presenter_;
};

} // namespace elda::views::admin_settings
