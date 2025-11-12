#pragma once
#include <memory>
#include "impedance_viewer_model.h"
#include "impedance_viewer_view.h"

namespace elda::impedance_viewer {

    class ImpedanceViewerPresenter {
    public:
        ImpedanceViewerPresenter(
            std::unique_ptr<ImpedanceViewerModel> model,
            std::unique_ptr<ImpedanceViewerView> view);

        void OnEnter();
        void OnExit();
        void Update(float dt);
        void Render(bool *isOpen);

    private:
        // callbacks bound once
        void SetupCallbacks();

        // handlers
        void OnElectrodeMouseDown(int idx); // -1 -> canvas
        void OnElectrodeDropped(size_t idx, ImVec2 normalizedPos);

        std::unique_ptr<ImpedanceViewerModel> model_;
        std::unique_ptr<ImpedanceViewerView> view_;
        ImpedanceViewerViewCallbacks callbacks_;
    };

} // namespace elda::impedance_viewer
