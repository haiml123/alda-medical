#include "impedance_viewer_presenter.h"
#include "imgui.h"
#include <iostream>

namespace elda::impedance_viewer {

    ImpedanceViewerPresenter::ImpedanceViewerPresenter(
        std::unique_ptr<ImpedanceViewerModel> model,
        std::unique_ptr<ImpedanceViewerView> view)
        : model_(std::move(model))
        , view_(std::move(view))
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
        // no-op for now
    }

    void ImpedanceViewerPresenter::Render(bool* isOpen) {
        view_->Render(
            isOpen,
            model_->GetElectrodePositions(),
            model_->GetAvailableChannels(),
            model_->GetSelectedElectrodeIndex(), callbacks_);
    }

    void ImpedanceViewerPresenter::SetupCallbacks() {
        callbacks_.onElectrodeMouseDown = [this](int idx){
            OnElectrodeMouseDown(idx);
        };
        callbacks_.onElectrodeDropped = [this](size_t idx, ImVec2 pos){
            OnElectrodeDropped(idx, pos);
        };
        callbacks_.onClose = [this](){
            model_->ClearSelection();
        };
    }

    void ImpedanceViewerPresenter::OnElectrodeMouseDown(int idx) {
        if (idx < 0) {
            // canvas click -> deselect
            model_->ClearSelection();
            return;
        }
        model_->SelectElectrode(idx);
        model_->StartDragging(static_cast<size_t>(idx));
    }

    void ImpedanceViewerPresenter::OnElectrodeDropped(size_t idx, ImVec2 normalizedPos) {
        model_->UpdateElectrodePosition(idx, normalizedPos.x, normalizedPos.y);
        model_->StopDragging(idx);
    }

} // namespace elda::impedance_viewer
