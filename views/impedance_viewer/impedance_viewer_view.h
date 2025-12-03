#pragma once
#include "imgui.h"
#include "impedance_viewer_model.h"
#include "UI/screen_header/screen_header.h"
#include <functional>
#include <vector>

namespace elda::views::impedance_viewer {

    struct ImpedanceViewerViewCallbacks {
        std::function<void(int electrode_index)> on_electrode_mouse_down;
        std::function<void(size_t electrode_index, ImVec2 normalized_drop_pos)> on_electrode_dropped;
        std::function<void()> on_save;
        std::function<void()> on_back;
        std::function<void()> on_settings;
        std::function<void()> on_monitoring;
    };

    struct ImpedanceViewerViewData {
        const std::vector<ElectrodePosition>* electrodes = nullptr;
        const std::vector<elda::models::Channel>* available_channels = nullptr;
        int selected_electrode_index = -1;
    };

    class ImpedanceViewerView {
    public:
        ImpedanceViewerView();

        void render(const ImpedanceViewerViewData& data,
                    const ImpedanceViewerViewCallbacks& callbacks);

    private:
        const float k_electrode_radius_px_   = 15.0f;
        const bool  k_show_grid_default_     = true;
        const float k_cap_radius_normalized_ = 0.40f;

        ImVec2 canvas_pos_{};
        ImVec2 canvas_size_{};
        ImVec2 center_pos_{};
        float  pixel_cap_radius_ = 0.0f;

        void render_body(const ImpedanceViewerViewData& data,
                         const ImpedanceViewerViewCallbacks& callbacks);

        void render_electrodes(ImDrawList* draw_list,
                               const std::vector<ElectrodePosition>& electrodes,
                               const std::vector<elda::models::Channel>& available_channels,
                               int selected_electrode_index,
                               const ImpedanceViewerViewCallbacks& callbacks);

        void render_single_electrode(ImDrawList* draw_list,
                                     size_t index,
                                     const ElectrodePosition& electrode,
                                     const elda::models::Channel* channel,
                                     bool is_selected,
                                     const ImpedanceViewerViewCallbacks& callbacks);
    };

}