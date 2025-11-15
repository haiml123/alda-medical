#include "impedance_viewer_presenter.h"
#include "imgui.h"
#include <iostream>

#include "UI/popup_message/popup_message.h"

namespace elda::views::impedance_viewer {

    ImpedanceViewerPresenter::ImpedanceViewerPresenter(
        ImpedanceViewerModel& model,
        ImpedanceViewerView& view,
        AppRouter& router)
        : router_(router)
        , model_(model)
        , view_(view)
    {
        setup_callbacks();
    }

    void ImpedanceViewerPresenter::on_enter() {
        std::cout << "[ImpedanceViewer] Enter\n";
    }

    void ImpedanceViewerPresenter::on_exit() {
        std::cout << "[ImpedanceViewer] Exit\n";
    }

    void ImpedanceViewerPresenter::update(float) {
        model_.update();
    }

    void ImpedanceViewerPresenter::render() {
        ImpedanceViewerViewData viewData;
        viewData.electrodes = &model_.get_electrode_positions();
        viewData.available_channels = &model_.get_available_channels();
        viewData.selected_electrode_index = model_.get_selected_electrode_index();

        view_.render(viewData, callbacks_);
    }

    void ImpedanceViewerPresenter::setup_callbacks() {
        callbacks_.on_electrode_mouse_down = [this](int idx){
            on_electrode_mouse_down(idx);
        };

        callbacks_.on_electrode_dropped = [this](size_t idx, ImVec2 pos){
            on_electrode_dropped(idx, pos);
        };

        callbacks_.on_save = [this](){
            on_save();
        };

        callbacks_.on_redirect_to_monitoring = [this](){
            // TODO save impedance values
       //      elda::ui::PopupMessage::Instance().Show(
       //     "Low Impedance Warning",
       //     "Some channels have impedance below 50kΩ. Are you sure you want to proceed to monitoring?",
       //     [this]() {
       //     router_.transitionTo(AppMode::MONITORING);
       //     }
       // );
            router_.transition_to(AppMode::MONITORING);
        };

        callbacks_.on_redirect_to_settings = [this](){
            router_.transition_to(AppMode::SETTINGS);
        };
    }

    void ImpedanceViewerPresenter::on_electrode_mouse_down(int idx) {
        if (idx < 0) {
            model_.clear_selection();
            return;
        }
        model_.select_electrode(idx);
        model_.start_dragging(static_cast<size_t>(idx));
    }

    void ImpedanceViewerPresenter::on_electrode_dropped(size_t idx, ImVec2 normalized_pos) {
        model_.update_electrode_position(idx, normalized_pos.x, normalized_pos.y);
        model_.stop_dragging(idx);
    }

    void ImpedanceViewerPresenter::on_save() {
        std::cout << "[ImpedanceViewer] Save button clicked\n";
        model_.save_positions_to_state();
    }

    void ImpedanceViewerPresenter::on_close() {
        std::cout << "[ImpedanceViewer] Close button clicked\n";
        model_.discard_changes();
        model_.clear_selection();
    }

} // namespace elda::impedance_viewer
