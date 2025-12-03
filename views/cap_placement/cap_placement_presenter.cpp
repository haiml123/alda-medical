#include "cap_placement_presenter.h"
#include <iostream>

namespace elda::views::cap_placement {

CapPlacementPresenter::CapPlacementPresenter(CapPlacementModel& model,
                                               CapPlacementView& view,
                                               AppRouter& router,
                                               elda::AppStateManager& state_manager)
    : model_(model)
    , view_(view)
    , router_(router)
    , state_manager_(state_manager)
{
    setup_callbacks();
}

void CapPlacementPresenter::on_enter() {
    std::cout << "[CapPlacement] Enter\n";
    model_.reset();
    animation_time_ = 0.0f;
}

void CapPlacementPresenter::on_exit() {
    std::cout << "[CapPlacement] Exit\n";
}

void CapPlacementPresenter::update(float delta_time) {
    animation_time_ += delta_time;
}

void CapPlacementPresenter::render() {
    CapPlacementViewData view_data;
    view_data.current_step = &model_.current_step();
    view_data.current_index = model_.current_index();
    view_data.total_steps = model_.step_count();
    view_data.is_first_step = model_.is_first_step();
    view_data.is_last_step = model_.is_last_step();
    view_data.animation_progress = animation_time_;

    view_.render(view_data, callbacks_);
}

void CapPlacementPresenter::setup_callbacks() {
    callbacks_.on_proceed = [this]() {
        handle_proceed();
    };

    callbacks_.on_back = [this]() {
        std::cout << "[CapPlacement] Back\n";
        router_.transition_to(AppMode::USER_SETTINGS);
    };

    callbacks_.on_next = [this]() {
        handle_next();
    };

    callbacks_.on_previous = [this]() {
        handle_previous();
    };
}

void CapPlacementPresenter::handle_proceed() {
    std::cout << "[CapPlacement] Proceed - navigating to user settings\n";
    router_.transition_to(AppMode::IMPEDANCE_VIEWER);
}

void CapPlacementPresenter::handle_next() {
    if (model_.is_last_step()) {
        // On last step, "Finish" acts like proceed
        handle_proceed();
    } else {
        model_.next_step();
        animation_time_ = 0.0f;  // Reset animation for new step
        std::cout << "[CapPlacement] Next step: " << model_.current_index() + 1 << "\n";
    }
}

void CapPlacementPresenter::handle_previous() {
    model_.previous_step();
    animation_time_ = 0.0f;
    std::cout << "[CapPlacement] Previous step: " << model_.current_index() + 1 << "\n";
}

} // namespace elda::views::cap_placement