#include "tabbar.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace elda::ui {

TabBar::TabBar()
    : active_tab_index_(0)
    , hovered_tab_index_(-1) {
}

TabBar::TabBar(const TabBarStyle& style)
    : active_tab_index_(0)
    , style_(style)
    , hovered_tab_index_(-1) {
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void TabBar::set_tabs(const std::vector<Tab>& tabs) {
    tabs_ = tabs;
    if (active_tab_index_ >= static_cast<int>(tabs_.size())) {
        active_tab_index_ = std::max(0, static_cast<int>(tabs_.size()) - 1);
    }
    tab_bounds_.clear();
    for (auto& tab : tabs_) {
        tab.bounds = nullptr;
    }
}

void TabBar::set_active_tab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        active_tab_index_ = index;
    }
}

void TabBar::set_style(const TabBarStyle& style) {
    style_ = style;
}

void TabBar::add_tab(const Tab& tab) {
    tabs_.push_back(tab);
}

void TabBar::remove_tab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        tabs_.erase(tabs_.begin() + index);

        if (active_tab_index_ >= static_cast<int>(tabs_.size())) {
            active_tab_index_ = std::max(0, static_cast<int>(tabs_.size()) - 1);
        }

        tab_bounds_.clear();
        for (auto& tab : tabs_) {
            tab.bounds = nullptr;
        }
    }
}

void TabBar::clear() {
    tabs_.clear();
    active_tab_index_ = 0;
    tab_bounds_.clear();
}

void TabBar::set_badge(int tab_index, int badge) {
    if (tab_index >= 0 && tab_index < static_cast<int>(tabs_.size())) {
        tabs_[tab_index].badge = badge;
    }
}

void TabBar::set_tab_enabled(int tab_index, bool enabled) {
    if (tab_index >= 0 && tab_index < static_cast<int>(tabs_.size())) {
        tabs_[tab_index].enabled = enabled;
    }
}

const Tab* TabBar::get_tab(int index) const {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        return &tabs_[index];
    }
    return nullptr;
}

// ============================================================================
// TAB BOUNDS QUERIES
// ============================================================================

const TabBounds* TabBar::get_tab_bounds(int index) const {
    if (index >= 0 && index < static_cast<int>(tab_bounds_.size())) {
        return &tab_bounds_[index];
    }
    return nullptr;
}

int TabBar::get_tab_at_position(float x, float y) const {
    for (int i = 0; i < static_cast<int>(tab_bounds_.size()); ++i) {
        if (tab_bounds_[i].contains(x, y)) {
            return i;
        }
    }
    return -1;
}

// ============================================================================
// RENDERING
// ============================================================================

bool TabBar::render() {
    if (tabs_.empty()) return false;

    for (auto& tab : tabs_) tab.bounds = nullptr;
    tab_bounds_.clear();
    tab_bounds_.reserve(tabs_.size());

    bool tab_changed = false;

    ImGui::BeginChild("##TabBarContainer",
                      ImVec2(0, style_.height),
                      false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style_.spacing, 0));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style_.tab_bar_padding);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style_.tab_bar_padding);

    int new_hover = -1;

    for (size_t i = 0; i < tabs_.size(); ++i) {
        const bool is_active = (static_cast<int>(i) == active_tab_index_);

        render_tab(static_cast<int>(i), tabs_[i], is_active);

        if (i < tab_bounds_.size())
            tabs_[i].bounds = &tab_bounds_[i];

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (tabs_[i].enabled && !is_active) {
                active_tab_index_ = static_cast<int>(i);
                tab_changed = true;
                if (on_tab_click_) on_tab_click_(static_cast<int>(i), tabs_[i]);
            }
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
            if (tabs_[i].enabled && on_tab_double_click_)
                on_tab_double_click_(static_cast<int>(i), tabs_[i]);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (tabs_[i].enabled && on_tab_right_click_)
                on_tab_right_click_(static_cast<int>(i), tabs_[i]);
        }

        if (ImGui::IsItemHovered()) {
            new_hover = static_cast<int>(i);
        }

        if (i < tabs_.size() - 1)
            ImGui::SameLine();
    }

    if (!ImGui::IsWindowHovered()) {
        new_hover = -1;
    }

    if (hovered_tab_index_ != new_hover) {
        hovered_tab_index_ = new_hover;
        if (hovered_tab_index_ != -1 && on_tab_hover_)
            on_tab_hover_(hovered_tab_index_, tabs_[hovered_tab_index_]);
    }

    if (on_add_tab_ || style_.show_add_button) {
        ImGui::SameLine();
        render_add_button();
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();

    return tab_changed;
}

void TabBar::render_tab(int index, const Tab& tab, bool is_active) {
    const bool is_hovered = (hovered_tab_index_ == index);
    const bool is_disabled = !tab.enabled;

    char label[256];
    if (style_.show_badges && tab.badge >= 0) {
        std::snprintf(label, sizeof(label), "%s (%d)", tab.label.c_str(), tab.badge);
    } else {
        std::snprintf(label, sizeof(label), "%s", tab.label.c_str());
    }

    char button_id[300];
    std::snprintf(button_id, sizeof(button_id), "%s##tab_%d", label, index);

    ImVec2 size = calculate_tab_size(tab);

    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 text_size = ImGui::CalcTextSize(label);

    ImVec2 actual_size = size;
    if (actual_size.x == 0) {
        actual_size.x = text_size.x + style_.button_padding_x * 2;
    }
    if (actual_size.y == 0) {
        actual_size.y = style_.height - style_.tab_bar_padding;
    }

    tab_bounds_.emplace_back(cursor_pos.x, cursor_pos.y, actual_size.x, actual_size.y);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec4 bg_color;
    if (is_disabled) {
        bg_color = style_.disabled_color;
    } else if (is_active) {
        bg_color = style_.active_color;
    } else if (is_hovered) {
        bg_color = style_.hover_color;
    } else {
        bg_color = style_.inactive_color;
    }

    ImU32 bg_color_u32 = ImGui::GetColorU32(bg_color);
    ImU32 border_color_u32 = ImGui::GetColorU32(style_.border_color);

    if (style_.browser_style) {
        float rounding = style_.rounding;

        ImVec2 top_left     = cursor_pos;
        ImVec2 top_right    = ImVec2(cursor_pos.x + actual_size.x, cursor_pos.y);
        ImVec2 bottom_right = ImVec2(cursor_pos.x + actual_size.x, cursor_pos.y + actual_size.y);
        ImVec2 bottom_left  = ImVec2(cursor_pos.x, cursor_pos.y + actual_size.y);

        draw_list->PathClear();

        draw_list->PathLineTo(bottom_left);
        draw_list->PathLineTo(ImVec2(top_left.x, top_left.y + rounding));
        draw_list->PathArcTo(
            ImVec2(top_left.x + rounding, top_left.y + rounding),
            rounding, M_PI, M_PI * 1.5f, 8
        );
        draw_list->PathLineTo(ImVec2(top_right.x - rounding, top_right.y));
        draw_list->PathArcTo(
            ImVec2(top_right.x - rounding, top_right.y + rounding),
            rounding, M_PI * 1.5f, M_PI * 2.0f, 8
        );
        draw_list->PathLineTo(bottom_right);
        draw_list->PathFillConvex(bg_color_u32);

        if (style_.show_borders) {
            draw_list->AddLine(
                bottom_left,
                ImVec2(top_left.x, top_left.y + rounding),
                border_color_u32, style_.border_thickness
            );

            draw_list->PathClear();
            draw_list->PathArcTo(
                ImVec2(top_left.x + rounding, top_left.y + rounding),
                rounding, M_PI, M_PI * 1.5f, 8
            );
            draw_list->PathStroke(border_color_u32, false, style_.border_thickness);

            draw_list->AddLine(
                ImVec2(top_left.x + rounding, top_left.y),
                ImVec2(top_right.x - rounding, top_right.y),
                border_color_u32, style_.border_thickness
            );

            draw_list->PathClear();
            draw_list->PathArcTo(
                ImVec2(top_right.x - rounding, top_right.y + rounding),
                rounding, M_PI * 1.5f, M_PI * 2.0f, 8
            );
            draw_list->PathStroke(border_color_u32, false, style_.border_thickness);

            draw_list->AddLine(
                ImVec2(top_right.x, top_right.y + rounding),
                bottom_right,
                border_color_u32, style_.border_thickness
            );

            draw_list->AddLine(
                bottom_left, bottom_right,
                border_color_u32, style_.border_thickness
            );
        }
    } else {
        ImVec2 rect_min = cursor_pos;
        ImVec2 rect_max = ImVec2(cursor_pos.x + actual_size.x, cursor_pos.y + actual_size.y);

        draw_list->AddRectFilled(rect_min, rect_max, bg_color_u32, style_.rounding);

        if (style_.show_borders) {
            draw_list->AddRect(rect_min, rect_max, border_color_u32,
                               style_.rounding, 0, style_.border_thickness);
        }
    }

    ImGui::SetCursorScreenPos(cursor_pos);

    if (is_disabled) {
        ImGui::BeginDisabled();
    }

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::Button(button_id, actual_size);
    ImGui::PopStyleColor(3);

    ImVec2 text_pos = ImVec2(
        cursor_pos.x + (actual_size.x - text_size.x) * 0.5f,
        cursor_pos.y + (actual_size.y - text_size.y) * 0.5f
    );

    ImU32 text_color = is_disabled
        ? ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f))
        : ImGui::GetColorU32(ImGuiCol_Text);

    draw_list->AddText(text_pos, text_color, label);

    if (is_disabled) {
        ImGui::EndDisabled();
    }

    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + actual_size.x, cursor_pos.y));
}

void TabBar::render_add_button() {
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

    float button_size = style_.height - style_.tab_bar_padding;
    ImVec2 size(button_size, button_size);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 top_left     = cursor_pos;
    ImVec2 top_right    = ImVec2(cursor_pos.x + size.x, cursor_pos.y);
    ImVec2 bottom_right = ImVec2(cursor_pos.x + size.x, cursor_pos.y + size.y);
    ImVec2 bottom_left  = ImVec2(cursor_pos.x, cursor_pos.y + size.y);

    ImVec4 bg_color = style_.inactive_color;
    ImU32 bg_color_u32 = ImGui::GetColorU32(bg_color);
    ImU32 border_color_u32 = ImGui::GetColorU32(style_.border_color);

    float rounding = style_.rounding;

    if (style_.browser_style) {
        draw_list->PathLineTo(bottom_left);
        draw_list->PathLineTo(ImVec2(top_left.x, top_left.y + rounding));
        draw_list->PathArcTo(
            ImVec2(top_left.x + rounding, top_left.y + rounding),
            rounding, M_PI, M_PI * 1.5f, 8
        );
        draw_list->PathLineTo(ImVec2(top_right.x - rounding, top_right.y));
        draw_list->PathArcTo(
            ImVec2(top_right.x - rounding, top_right.y + rounding),
            rounding, M_PI * 1.5f, M_PI * 2.0f, 8
        );
        draw_list->PathLineTo(bottom_right);
        draw_list->PathFillConvex(bg_color_u32);

        if (style_.show_borders) {
            draw_list->PathClear();
            draw_list->PathArcTo(
                ImVec2(top_left.x + rounding, top_left.y + rounding),
                rounding, M_PI, M_PI * 1.5f, 8
            );
            draw_list->PathStroke(border_color_u32, false, style_.border_thickness);

            draw_list->AddLine(
                ImVec2(top_left.x + rounding, top_left.y),
                ImVec2(top_right.x - rounding, top_right.y),
                border_color_u32, style_.border_thickness
            );

            draw_list->PathClear();
            draw_list->PathArcTo(
                ImVec2(top_right.x - rounding, top_right.y + rounding),
                rounding, M_PI * 1.5f, M_PI * 2.0f, 8
            );
            draw_list->PathStroke(border_color_u32, false, style_.border_thickness);

            draw_list->AddLine(
                ImVec2(top_right.x, top_right.y + rounding),
                bottom_right,
                border_color_u32, style_.border_thickness
            );

            draw_list->AddLine(
                bottom_left, bottom_right,
                border_color_u32, style_.border_thickness
            );
        }
    } else {
        draw_list->AddRectFilled(top_left, bottom_right, bg_color_u32, rounding);
        if (style_.show_borders) {
            draw_list->AddRect(top_left, bottom_right, border_color_u32,
                               rounding, 0, style_.border_thickness);
        }
    }

    ImGui::SetCursorScreenPos(cursor_pos);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 0.9f, 0.3f));

    if (ImGui::Button("##AddTab", size)) {
        if (on_add_tab_) {
            on_add_tab_();
        }
    }

    ImGui::PopStyleColor(3);

    ImVec2 center(cursor_pos.x + size.x * 0.5f, cursor_pos.y + size.y * 0.5f);
    float plus_size = size.x * style_.add_button_icon_size;
    ImU32 plus_color = ImGui::GetColorU32(ImGuiCol_Text);
    float thickness = 2.0f;

    draw_list->AddLine(
        ImVec2(center.x - plus_size, center.y),
        ImVec2(center.x + plus_size, center.y),
        plus_color, thickness
    );

    draw_list->AddLine(
        ImVec2(center.x, center.y - plus_size),
        ImVec2(center.x, center.y + plus_size),
        plus_color, thickness
    );

    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + size.x, cursor_pos.y));
}

void TabBar::render_badge(int badge_count) {
    if (badge_count < 0) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMax();
    pos.x -= 20.0f;
    pos.y += 4.0f;

    char badge[16];
    std::snprintf(badge, sizeof(badge), "%d", badge_count);

    ImVec2 text_size = ImGui::CalcTextSize(badge);
    float radius = std::max(text_size.x, text_size.y) * 0.5f + 4.0f;

    draw_list->AddCircleFilled(pos, radius, ImGui::GetColorU32(style_.badge_color));
    draw_list->AddText(
        ImVec2(pos.x - text_size.x * 0.5f, pos.y - text_size.y * 0.5f),
        ImGui::GetColorU32(style_.badge_text_color),
        badge
    );
}

ImVec2 TabBar::calculate_tab_size(const Tab& tab) const {
    char label[256];
    if (style_.show_badges && tab.badge >= 0) {
        std::snprintf(label, sizeof(label), "%s (%d)", tab.label.c_str(), tab.badge);
    } else {
        std::snprintf(label, sizeof(label), "%s", tab.label.c_str());
    }

    ImVec2 text_size = ImGui::CalcTextSize(label);

    if (style_.auto_size) {
        float available_width = ImGui::GetContentRegionAvail().x - (style_.tab_bar_padding * 2);
        float total_spacing = style_.spacing * (tabs_.size() - 1);
        float tab_width = (available_width - total_spacing) / tabs_.size();
        return ImVec2(tab_width, text_size.y + style_.button_padding_y * 2);
    }

    return ImVec2(0, 0);
}

void TabBar::apply_tab_style(bool is_active, bool /*is_hovered*/, bool is_disabled) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style_.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(style_.button_padding_x, style_.button_padding_y));

    ImVec4 bg_color;
    if (is_disabled) {
        bg_color = style_.disabled_color;
    } else if (is_active) {
        bg_color = style_.active_color;
    } else {
        bg_color = style_.inactive_color;
    }

    ImVec4 hover_color = is_active ? style_.active_color : style_.hover_color;

    ImGui::PushStyleColor(ImGuiCol_Button, bg_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, hover_color);
}

void TabBar::restore_style() {
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

} // namespace elda::ui
