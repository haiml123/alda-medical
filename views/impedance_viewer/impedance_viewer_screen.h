#pragma once
#include <memory>
#include "core/core.h"
#include "core/app_state_manager.h"
#include "impedance_viewer_presenter.h"
#include "impedance_viewer_model.h"
#include "impedance_viewer_view.h"

namespace elda::impedance_viewer {

    class ImpedanceViewerScreen {
    public:
        ImpedanceViewerScreen(AppState& state, AppStateManager& stateManager);

        void onEnter();
        void onExit();
        void update(float dt);
        void render();
        bool isOpen() const { return isOpen_; }

    private:
        bool isOpen_ = true;
        std::unique_ptr<ImpedanceViewerPresenter> presenter_;
    };

} // namespace elda::impedance_viewer
