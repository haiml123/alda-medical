#include "cap_placement_view.h"
#include <cmath>

namespace elda::views::cap_placement {

void CapPlacementView::render(const CapPlacementViewData& data,
                               const CapPlacementViewCallbacks& callbacks) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##CapPlacementScreen", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header using reusable component
    char step_text[32];
    snprintf(step_text, sizeof(step_text), "%zu / %zu",
             data.current_index + 1, data.total_steps);

    elda::ui::render_screen_header({
        .title = "Cap Placement Guide",
        .show_back_button = true,
        .on_back = callbacks.on_back,
        .buttons = {
            {
                .label = step_text,
                .on_click = nullptr,
                .enabled = false,  // Just display, not clickable
                .primary = false,
                .width = 60.0f
            },
            {
                .label = "CAP PLACEMENT",
                .on_click = callbacks.on_proceed,
                .enabled = true,
                .primary = true,
                .width = 140.0f
            }
        }
    });

    // Content
    render_content(data, callbacks);

    ImGui::End();
    ImGui::PopStyleVar();
}

void CapPlacementView::render_content(const CapPlacementViewData& data,
                                       const CapPlacementViewCallbacks& callbacks) {
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
    ImGui::BeginChild("##Content", content_size, false);

    const float max_width = 800.0f;
    float available_width = ImGui::GetContentRegionAvail().x;
    float padding = (available_width > max_width)
                    ? (available_width - max_width) / 2.0f
                    : 40.0f;

    ImGui::Dummy(ImVec2(0, 40));  // Top padding

    ImGui::SetCursorPosX(padding);
    ImGui::BeginGroup();

    // Step indicator dots
    render_step_indicator(data);

    ImGui::Dummy(ImVec2(0, 30));

    // Animation/illustration placeholder
    render_animation_placeholder(data);

    ImGui::Dummy(ImVec2(0, 30));

    // Step content (title and description)
    render_step_content(data);

    ImGui::Dummy(ImVec2(0, 40));

    // Navigation buttons
    render_navigation(data, callbacks);

    ImGui::EndGroup();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void CapPlacementView::render_step_indicator(const CapPlacementViewData& data) {
    const float dot_radius = 6.0f;
    const float dot_spacing = 24.0f;
    const float total_width = (data.total_steps - 1) * dot_spacing + dot_radius * 2;

    // Center the dots
    float start_x = (800.0f - total_width) / 2.0f;

    ImVec2 cursor_start = ImGui::GetCursorScreenPos();
    cursor_start.x += start_x;
    cursor_start.y += dot_radius;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < data.total_steps; i++) {
        ImVec2 center(cursor_start.x + i * dot_spacing, cursor_start.y);

        if (i == data.current_index) {
            // Active dot - teal
            draw_list->AddCircleFilled(center, dot_radius,
                IM_COL32(77, 200, 205, 255));
        } else if (i < data.current_index) {
            // Completed dot - dimmer teal
            draw_list->AddCircleFilled(center, dot_radius,
                IM_COL32(77, 200, 205, 150));
        } else {
            // Future dot - gray
            draw_list->AddCircleFilled(center, dot_radius,
                IM_COL32(80, 85, 90, 255));
        }
    }

    ImGui::Dummy(ImVec2(0, dot_radius * 2 + 8));
}

void CapPlacementView::render_animation_placeholder(const CapPlacementViewData& data) {
    const float box_width = 800.0f;
    const float box_height = 300.0f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Background box
    draw_list->AddRectFilled(
        pos,
        ImVec2(pos.x + box_width, pos.y + box_height),
        IM_COL32(20, 22, 25, 255),
        8.0f
    );

    // Border
    draw_list->AddRect(
        pos,
        ImVec2(pos.x + box_width, pos.y + box_height),
        IM_COL32(50, 55, 60, 255),
        8.0f
    );

    // Animated circle to show this is an animation area
    float time = static_cast<float>(ImGui::GetTime());
    float pulse = (std::sin(time * 2.0f) + 1.0f) * 0.5f;  // 0 to 1

    ImVec2 center(pos.x + box_width / 2, pos.y + box_height / 2);
    float base_radius = 40.0f;
    float animated_radius = base_radius + pulse * 10.0f;

    // Outer glow
    draw_list->AddCircleFilled(center, animated_radius + 20.0f,
        IM_COL32(77, 200, 205, static_cast<int>(30 * pulse)));

    // Main circle
    draw_list->AddCircleFilled(center, animated_radius,
        IM_COL32(77, 200, 205, 180));

    // Step number in circle
    char step_num[8];
    snprintf(step_num, sizeof(step_num), "%zu", data.current_index + 1);
    ImVec2 text_size = ImGui::CalcTextSize(step_num);

    draw_list->AddText(
        ImVec2(center.x - text_size.x / 2, center.y - text_size.y / 2),
        IM_COL32(255, 255, 255, 255),
        step_num
    );

    // Placeholder text
    const char* placeholder = "Animation / Illustration";
    ImVec2 ph_size = ImGui::CalcTextSize(placeholder);
    draw_list->AddText(
        ImVec2(center.x - ph_size.x / 2, pos.y + box_height - 40),
        IM_COL32(100, 105, 110, 255),
        placeholder
    );

    ImGui::Dummy(ImVec2(box_width, box_height));
}

void CapPlacementView::render_step_content(const CapPlacementViewData& data) {
    if (!data.current_step) return;

    const float content_width = 800.0f;

    // Title - centered
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.92f, 0.95f, 1.0f));

    float title_width = ImGui::CalcTextSize(data.current_step->title.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (content_width - title_width) / 2.0f);
    ImGui::Text("%s", data.current_step->title.c_str());

    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0, 16));

    // Description - centered text block
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.68f, 0.72f, 1.0f));

    // Calculate wrapped text size to center it
    const float max_text_width = 600.0f;
    float text_offset = (content_width - max_text_width) / 2.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + text_offset);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + max_text_width);

    // Center-align by using a child window
    ImGui::BeginGroup();
    ImGui::Text("%s", data.current_step->description.c_str());
    ImGui::EndGroup();

    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

void CapPlacementView::render_navigation(const CapPlacementViewData& data,
                                          const CapPlacementViewCallbacks& callbacks) {
    const float btn_width = 120.0f;
    const float btn_height = 40.0f;
    const float spacing = 16.0f;

    const ImVec4 gray   = ImVec4(0.20f, 0.21f, 0.23f, 1.0f);
    const ImVec4 gray_h = ImVec4(0.25f, 0.26f, 0.28f, 1.0f);
    const ImVec4 teal   = ImVec4(0.20f, 0.65f, 0.70f, 1.0f);
    const ImVec4 teal_h = ImVec4(0.25f, 0.75f, 0.80f, 1.0f);

    // Center the buttons
    float total_width = btn_width * 2 + spacing;
    float start_x = (800.0f - total_width) / 2.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

    // Previous button
    ImGui::PushStyleColor(ImGuiCol_Button, gray);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, gray_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, gray);

    bool prev_disabled = data.is_first_step;
    if (prev_disabled) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button("< Previous", ImVec2(btn_width, btn_height)) && !prev_disabled) {
        if (callbacks.on_previous) {
            callbacks.on_previous();
        }
    }

    if (prev_disabled) {
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, spacing);

    // Next button
    ImGui::PushStyleColor(ImGuiCol_Button, teal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, teal_h);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, teal);

    const char* next_label = data.is_last_step ? "Finish" : "Next >";

    if (ImGui::Button(next_label, ImVec2(btn_width, btn_height))) {
        if (callbacks.on_next) {
            callbacks.on_next();
        }
    }

    ImGui::PopStyleColor(3);
}

} // namespace elda::views::cap_placement