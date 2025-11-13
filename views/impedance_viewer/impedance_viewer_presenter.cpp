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
            model_->GetSelectedElectrodeIndex(),
            callbacks_);
    }

    void ImpedanceViewerPresenter::SetupCallbacks() {
        callbacks_.onElectrodeMouseDown = [this](int idx){
            OnElectrodeMouseDown(idx);
        };

        callbacks_.onElectrodeDropped = [this](size_t idx, ImVec2 pos){
            OnElectrodeDropped(idx, pos);
        };

        // ✅ NEW: Save button callback
        callbacks_.onSave = [this](){
            OnSave();
        };

        // ✅ NEW: Close button callback (discard changes)
        callbacks_.onClose = [this](){
            OnClose();
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
        // ✅ Update temporary position (not saved to Channel model yet)
        model_->UpdateElectrodePosition(idx, normalizedPos.x, normalizedPos.y);
        model_->StopDragging(idx);
    }

    void ImpedanceViewerPresenter::OnSave() {
        std::cout << "[ImpedanceViewer] Save button clicked\n";
        // ✅ Persist all temporary position changes to Channel models in state
        model_->SavePositionsToState();
    }

    void ImpedanceViewerPresenter::OnClose() {
        std::cout << "[ImpedanceViewer] Close button clicked\n";
        // ✅ Discard all temporary changes and restore original positions
        model_->DiscardChanges();
        model_->ClearSelection();
    }

} // namespace elda::impedance_viewer