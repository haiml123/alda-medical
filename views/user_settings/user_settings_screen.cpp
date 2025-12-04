#include "user_settings_screen.h"

#include "core/app_state_manager.h"
#include "core/core.h"

namespace elda::views::user_settings
{

UserSettingsScreen::UserSettingsScreen(AppState& /*state*/, AppStateManager& state_manager, AppRouter& router)
    : presenter_(model_, view_, router, state_manager)
{
}

void UserSettingsScreen::on_enter()
{
    presenter_.on_enter();
}

void UserSettingsScreen::on_exit()
{
    presenter_.on_exit();
}

void UserSettingsScreen::update(float delta_time)
{
    presenter_.update(delta_time);
}

void UserSettingsScreen::render()
{
    presenter_.render();
}

}  // namespace elda::views::user_settings
