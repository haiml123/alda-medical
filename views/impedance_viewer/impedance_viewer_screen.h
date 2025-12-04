#pragma once
#include "core/app_state_manager.h"
#include "core/core.h"
#include "core/router/app_router.h"
#include "impedance_viewer_model.h"
#include "impedance_viewer_presenter.h"
#include "impedance_viewer_view.h"

namespace elda::views::impedance_viewer
{

class ImpedanceViewerScreen : public IScreen
{
  public:
    ImpedanceViewerScreen(AppState& state, AppStateManager& state_manager, AppRouter& router);

    void on_enter() override;
    void on_exit() override;
    void update(float dt) override;
    void render() override;

  private:
    ImpedanceViewerModel model_;
    ImpedanceViewerView view_;
    ImpedanceViewerPresenter presenter_;
};

}  // namespace elda::views::impedance_viewer
