#include "impedance_viewer_presenter.h"
#include "imgui.h"
#include <iostream>

#include "UI/popup_message/popup_message.h"

namespace elda::impedance_viewer {

    ImpedanceViewerPresenter::ImpedanceViewerPresenter(
        ImpedanceViewerModel& model,
        ImpedanceViewerView& view,
        AppRouter& router)
        : router_(router)
        , model_(model)
        , view_(view)
    {
        SetupCallbacks();
    }

    void ImpedanceViewerPresenter::OnEnter() {
        std::cout << "[ImpedanceViewer] Enter\n";
    }

    void ImpedanceViewerPresenter::OnExit() {
        std::cout << "[ImpedanceViewer] Exit\n";
    }

    void ImpedanceViewerPresenter::Update(float) {
        model_.Update();
    }

    void ImpedanceViewerPresenter::Render() {
        ImpedanceViewerViewData viewData;
        viewData.electrodes = &model_.GetElectrodePositions();
        viewData.availableChannels = &model_.GetAvailableChannels();
        viewData.selectedElectrodeIndex = model_.GetSelectedElectrodeIndex();

        view_.Render(viewData, callbacks_);
    }

    void ImpedanceViewerPresenter::SetupCallbacks() {
        callbacks_.onElectrodeMouseDown = [this](int idx){
            OnElectrodeMouseDown(idx);
        };

        callbacks_.onElectrodeDropped = [this](size_t idx, ImVec2 pos){
            OnElectrodeDropped(idx, pos);
        };

        callbacks_.onSave = [this](){
            OnSave();
        };

        callbacks_.onRedirectToMonitoring = [this](){
            // TODO save impedance values
       //      elda::ui::PopupMessage::Instance().Show(
       //     "Low Impedance Warning",
       //     "Some channels have impedance below 50kΩ. Are you sure you want to proceed to monitoring?",
       //     [this]() {
       router_.transitionTo(AppMode::MONITORING);
       //     }
       // );

        };

        callbacks_.onRedirectToSettings = [this](){
            router_.transitionTo(AppMode::SETTINGS);
        };
    }

    void ImpedanceViewerPresenter::OnElectrodeMouseDown(int idx) {
        if (idx < 0) {
            model_.ClearSelection();
            return;
        }
        model_.SelectElectrode(idx);
        model_.StartDragging(static_cast<size_t>(idx));
    }

    void ImpedanceViewerPresenter::OnElectrodeDropped(size_t idx, ImVec2 normalizedPos) {
        model_.UpdateElectrodePosition(idx, normalizedPos.x, normalizedPos.y);
        model_.StopDragging(idx);
    }

    void ImpedanceViewerPresenter::OnSave() {
        std::cout << "[ImpedanceViewer] Save button clicked\n";
        model_.SavePositionsToState();
    }

    void ImpedanceViewerPresenter::OnClose() {
        std::cout << "[ImpedanceViewer] Close button clicked\n";
        model_.DiscardChanges();
        model_.ClearSelection();
    }

} // namespace elda::impedance_viewer