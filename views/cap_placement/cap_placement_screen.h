#pragma once

#include "cap_placement_model.h"
#include "cap_placement_presenter.h"
#include "cap_placement_view.h"
#include "core/app_state_manager.h"
#include "core/router/IScreen.h"
#include "core/router/app_router.h"

namespace elda::views::cap_placement
{

class CapPlacementScreen : public IScreen
{
  public:
    CapPlacementScreen(AppState& app_state, elda::AppStateManager& state_manager, AppRouter& router)
        : app_state_(app_state), presenter_(model_, view_, router, state_manager)
    {
    }

    void on_enter() override
    {
        presenter_.on_enter();
    }

    void on_exit() override
    {
        presenter_.on_exit();
    }

    void update(float delta_time) override
    {
        presenter_.update(delta_time);
    }

    void render() override
    {
        presenter_.render();
    }

  private:
    AppState& app_state_;
    CapPlacementModel model_;
    CapPlacementView view_;
    CapPlacementPresenter presenter_;
};

}  // namespace elda::views::cap_placement
