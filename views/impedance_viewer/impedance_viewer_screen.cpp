#include "impedance_viewer_screen.h"

namespace elda::views::impedance_viewer
{

ImpedanceViewerScreen::ImpedanceViewerScreen(AppState& state, AppStateManager& state_manager, AppRouter& router)
    : model_(state.available_channels ? *state.available_channels : std::vector<elda::models::Channel>{},
             state_manager),
      view_(),
      presenter_(model_, view_, router)
{
}

void ImpedanceViewerScreen::on_enter()
{
    presenter_.on_enter();
}

void ImpedanceViewerScreen::on_exit()
{
    presenter_.on_exit();
}

void ImpedanceViewerScreen::update(float dt)
{
    presenter_.update(dt);
}

void ImpedanceViewerScreen::render()
{
    presenter_.render();
}

}  // namespace elda::views::impedance_viewer
