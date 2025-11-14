#pragma once
#include "core/core.h"
#include "core/app_state_manager.h"
#include "impedance_viewer_presenter.h"
#include "impedance_viewer_model.h"
#include "impedance_viewer_view.h"
#include "core/router/app_router.h"

namespace elda::impedance_viewer {

    class ImpedanceViewerScreen : public IScreen {
    public:
        ImpedanceViewerScreen(AppState& state, AppStateManager& stateManager, AppRouter& router);

        void onEnter() override;
        void onExit() override;
        void update(float dt) override;
        void render() override;

    private:
        ImpedanceViewerModel model_;
        ImpedanceViewerView view_;
        ImpedanceViewerPresenter presenter_;
    };

} // namespace elda::impedance_viewer