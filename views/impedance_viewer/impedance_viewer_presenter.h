#pragma once
#include "core/router/app_router.h"
#include "impedance_viewer_model.h"
#include "impedance_viewer_view.h"

namespace elda::views::impedance_viewer
{

class ImpedanceViewerPresenter
{
  public:
    ImpedanceViewerPresenter(ImpedanceViewerModel& model, ImpedanceViewerView& view, AppRouter& router);

    void on_enter();
    void on_exit();
    void update(float delta_time);
    void render();

  private:
    void setup_callbacks();

    void on_electrode_mouse_down(int idx);
    void on_electrode_dropped(size_t idx, ImVec2 normalized_pos);
    void on_save();
    void on_close();

    AppRouter& router_;
    ImpedanceViewerModel& model_;
    ImpedanceViewerView& view_;
    ImpedanceViewerViewCallbacks callbacks_;
};

}  // namespace elda::views::impedance_viewer