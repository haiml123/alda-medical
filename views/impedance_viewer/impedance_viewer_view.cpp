#include "impedance_viewer_view.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cmath>

#include "views/impedance_viewer/impedance_viewer_helper.h"
#include "UI/impedance_range/impedance_range.h"
#include "UI/screen_header/screen_header.h"

namespace elda::views::impedance_viewer {

ImpedanceViewerView::ImpedanceViewerView() = default;

void ImpedanceViewerView::render(const ImpedanceViewerViewData& data,
                                 const ImpedanceViewerViewCallbacks& callbacks)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("ImpedanceViewer", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header using reusable component
    ui::render_screen_header({
        .title = "Impedance Setup",
        .show_back_button = true,
        .on_back = callbacks.on_back,
        .buttons = {
            {
                .label = "SETTINGS",
                .on_click = callbacks.on_settings,
                .enabled = true,
                .primary = false,
                .width = 100.0f
            },
            {
                .label = "MONITORING",
                .on_click = callbacks.on_monitoring,
                .enabled = true,
                .primary = true,
                .width = 120.0f
            }
        }
    });

    render_body(data, callbacks);

    ImGui::End();
    ImGui::PopStyleVar();
}

void ImpedanceViewerView::render_body(const ImpedanceViewerViewData& data,
                                      const ImpedanceViewerViewCallbacks& callbacks)
{
    constexpr float range_panel_height = 110.0f;
    const float available_cap_height = ImGui::GetContentRegionAvail().y - range_panel_height;

    canvas_pos_  = ImGui::GetCursorScreenPos();
    canvas_size_ = ImGui::GetContentRegionAvail();

    center_pos_  = ImVec2(
        canvas_pos_.x + canvas_size_.x * 0.5f,
        canvas_pos_.y + available_cap_height * 0.5f
    );

    pixel_cap_radius_ = std::min(canvas_size_.x, available_cap_height) * k_cap_radius_normalized_;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton("impedance_canvas",
        canvas_size_,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool canvas_hovered = ImGui::IsItemHovered();

    draw_cap_outline(draw_list, center_pos_, pixel_cap_radius_);
    if (k_show_grid_default_) draw_cap_grid(draw_list, center_pos_, pixel_cap_radius_);

    render_electrodes(draw_list,
                      *data.electrodes,
                      *data.available_channels,
                      data.selected_electrode_index,
                      callbacks);

    // background-click clears selection
    if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mp = ImGui::GetMousePos();
        bool hit = false;
        for (const auto& electrode : *data.electrodes) {
            ImVec2 p = cap_normalized_to_screen(center_pos_, pixel_cap_radius_, electrode.x, electrode.y);
            if (point_in_circle(mp, p, k_electrode_radius_px_)) { hit = true; break; }
        }
        if (!hit && callbacks.on_electrode_mouse_down)
            callbacks.on_electrode_mouse_down(-1);
    }

    //
    // bottom impedance range panel
    //
    {
        const float panel_padding = 40.0f;
        const float panel_width   = std::min(420.0f, canvas_size_.x * 0.60f);
        const float panel_height  = 86.0f;

        ImVec2 panel_pos(center_pos_.x - panel_width * 0.5f,
                         center_pos_.y + pixel_cap_radius_ + panel_padding);

        ImGui::SetCursorScreenPos(panel_pos);
        ImGui::BeginChild("impedance_range_panel",
                          ImVec2(panel_width, panel_height),
                          false,
                          ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse |
                          ImGuiWindowFlags_NoBackground);

        elda::ui::ImpedanceRanges ranges{ 10000.f, 30000.f, 54000.f };
        elda::ui::ImpedanceRangeConfig bar_cfg;
        bar_cfg.show_threshold_labels = true;

        elda::ui::DualCursorConfig cursor_cfg;
        cursor_cfg.cursor_color  = IM_COL32(0,0,0,255);
        cursor_cfg.min_gap_ohms  = 500.0f;
        cursor_cfg.draggable     = true;

        static float low_ohm  = 10000.f;
        static float high_ohm = 30000.f;

        elda::ui::draw_impedance_range_dual("imp-range-dual",
                                         low_ohm, high_ohm,
                                         ranges, bar_cfg, cursor_cfg,
                                         &low_ohm, &high_ohm);

        ImGui::EndChild();
    }
}

void ImpedanceViewerView::render_electrodes(
    ImDrawList* draw_list,
    const std::vector<ElectrodePosition>& electrodes,
    const std::vector<elda::models::Channel>& available_channels,
    int selected_electrode_index,
    const ImpedanceViewerViewCallbacks& callbacks)
{
    for (size_t i = 0; i < electrodes.size(); ++i) {
        const auto& electrode = electrodes[i];

        const elda::models::Channel* channel = nullptr;
        if (!electrode.channel_id.empty()) {
            auto it = std::find_if(
                available_channels.begin(), available_channels.end(),
                [&](const elda::models::Channel& c){ return c.id == electrode.channel_id; }
            );
            if (it != available_channels.end()) channel = &(*it);
        }

        bool is_selected = (static_cast<int>(i) == selected_electrode_index);

        render_single_electrode(draw_list,
                                i,
                                electrode,
                                channel,
                                is_selected,
                                callbacks);
    }
}

void ImpedanceViewerView::render_single_electrode(
    ImDrawList* draw_list,
    size_t index,
    const ElectrodePosition& electrode,
    const elda::models::Channel* channel,
    bool is_selected,
    const ImpedanceViewerViewCallbacks& callbacks)
{
    ImVec2 pos = cap_normalized_to_screen(center_pos_, pixel_cap_radius_, electrode.x, electrode.y);
    ImVec2 mouse_pos = ImGui::GetMousePos();

    bool hovered = point_in_circle(mouse_pos, pos, k_electrode_radius_px_);

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !electrode.is_dragging) {
        if (callbacks.on_electrode_mouse_down)
            callbacks.on_electrode_mouse_down(static_cast<int>(index));
    }

    // dragging
    if (electrode.is_dragging) {
        ImVec2 drop_norm = screen_to_cap_normalized_clamped(center_pos_, pixel_cap_radius_, mouse_pos);
        pos = cap_normalized_to_screen(center_pos_, pixel_cap_radius_, drop_norm.x, drop_norm.y);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (callbacks.on_electrode_dropped)
                callbacks.on_electrode_dropped(index, drop_norm);
        }
    }

    //
    // circle color
    //
    ImU32 color_base = channel_color_from_id(channel ? channel->id : std::string());

    if (hovered) {
        const int r = (color_base      & 0xFF) + 15;
        const int g = ((color_base>>8) & 0xFF) + 15;
        const int b = ((color_base>>16)& 0xFF) + 15;
        color_base = IM_COL32(std::min(r,255), std::min(g,255), std::min(b,255), 255);
    }
    if (is_selected) {
        color_base = IM_COL32(255,235,150,255);
    }

    float radius = k_electrode_radius_px_;
    if (is_selected) radius *= 1.20f;
    if (hovered)     radius *= 1.10f;

    draw_list->AddCircleFilled(pos, radius, color_base);
    draw_list->AddCircle(pos, radius, IM_COL32(0,0,0,255), 22, 1.0f);

    //
    // label
    //
    const char* label = nullptr;
    char fallback[8];

    if (channel) label = channel->name.c_str();
    else {
        std::snprintf(fallback, sizeof(fallback), "%zu", index + 1);
        label = fallback;
    }

    const ImVec2 label_size = ImGui::CalcTextSize(label);
    draw_list->AddText(ImVec2(pos.x - label_size.x * 0.5f,
                              pos.y - label_size.y * 0.5f),
                       IM_COL32(0,0,0,255),
                       label);
}

}
