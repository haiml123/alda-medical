#pragma once
#include "impedance_viewer_model.h"
#include "impedance_viewer_view.h"
#include "core/router/app_router.h"

namespace elda::impedance_viewer {

    class ImpedanceViewerPresenter {
    public:
        ImpedanceViewerPresenter(
            ImpedanceViewerModel& model,
            ImpedanceViewerView& view,
            AppRouter& router);

        void OnEnter();
        void OnExit();
        void Update(float dt);
        void Render();

    private:
        void SetupCallbacks();

        void OnElectrodeMouseDown(int idx);
        void OnElectrodeDropped(size_t idx, ImVec2 normalizedPos);
        void OnSave();
        void OnClose();

        AppRouter& router_;
        ImpedanceViewerModel& model_;
        ImpedanceViewerView& view_;
        ImpedanceViewerViewCallbacks callbacks_;
    };

} // namespace elda::impedance_viewer