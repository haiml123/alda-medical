#pragma once

#include "core/app_state_manager.h"
#include "core/core.h"
#include "core/router/IScreen.h"
#include "core/router/app_router.h"
#include "user_settings_model.h"
#include "user_settings_presenter.h"
#include "user_settings_view.h"

namespace elda::views::user_settings
{

class UserSettingsScreen : public IScreen
{
  public:
    UserSettingsScreen(AppState& state, AppStateManager& state_manager, AppRouter& router);

    void on_enter() override;
    void on_exit() override;
    void update(float delta_time) override;
    void render() override;

  private:
    UserSettingsModel model_;
    UserSettingsView view_;
    UserSettingsPresenter presenter_;
};

}  // namespace elda::views::user_settings
