#pragma once

#include "admin_settings_model.h"
#include "admin_settings_view.h"
#include "core/app_state_manager.h"
#include "core/router/app_router.h"

namespace elda::views::admin_settings
{

class AdminSettingsPresenter
{
  public:
    AdminSettingsPresenter(AdminSettingsModel& model,
                           AdminSettingsView& view,
                           AppRouter& router,
                           AppStateManager& state_manager);

    void on_enter();
    void on_exit();
    void update(float delta_time);
    void render();

  private:
    void setup_callbacks();
    void sync_form_to_model();
    void handle_save();
    void handle_close();

    AdminSettingsModel& model_;
    AdminSettingsView& view_;
    AppRouter& router_;
    AppStateManager& state_manager_;
    AdminSettingsViewCallbacks callbacks_;
};

}  // namespace elda::views::admin_settings
