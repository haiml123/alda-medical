#pragma once

#include "user_settings_model.h"
#include "user_settings_view.h"
#include "core/router/app_router.h"
#include "core/app_state_manager.h"

namespace elda::views::user_settings {

    class UserSettingsPresenter {
    public:
        UserSettingsPresenter(UserSettingsModel& model,
                              UserSettingsView& view,
                              AppRouter& router,
                              AppStateManager& state_manager);

        void on_enter();
        void on_exit();
        void update(float delta_time);
        void render();

    private:
        void setup_callbacks();
        void sync_form_to_model();
        void handle_proceed();

        UserSettingsModel& model_;
        UserSettingsView& view_;
        AppRouter& router_;
        AppStateManager& state_manager_;
        UserSettingsViewCallbacks callbacks_;
    };

} // namespace elda::views::user_settings