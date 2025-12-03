#pragma once

#include "cap_placement_model.h"
#include "cap_placement_view.h"
#include "core/router/app_router.h"
#include "core/app_state_manager.h"

namespace elda::views::cap_placement {

    class CapPlacementPresenter {
    public:
        CapPlacementPresenter(CapPlacementModel& model,
                              CapPlacementView& view,
                              AppRouter& router,
                              elda::AppStateManager& state_manager);

        void on_enter();
        void on_exit();
        void update(float delta_time);
        void render();

    private:
        void setup_callbacks();
        void handle_proceed();
        void handle_next();
        void handle_previous();

        CapPlacementModel& model_;
        CapPlacementView& view_;
        AppRouter& router_;
        AppStateManager& state_manager_;

        CapPlacementViewCallbacks callbacks_;

        float animation_time_ = 0.0f;
    };

} // namespace elda::views::cap_placement