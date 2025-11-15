#include "channels_group_view.h"
#include "imgui_internal.h"
#include <cstring>
#include <cstdio>

namespace elda::views::channels_selector {

    ChannelsGroupView::ChannelsGroupView()
        : is_visible_(false)
        , position_(0, 0)
        , size_(300, 550) {
        name_input_buffer_[0] = '\0';
    }

    // ========================================================================
    // RENDERING
    // ========================================================================

    void ChannelsGroupView::render(
        const std::string& group_name,
        const std::vector<models::Channel>& channels,
        int selected_count,
        int total_count,
        bool can_confirm,
        bool is_new_group
    ) {
        if (!is_visible_) return;

        ImGui::SetNextWindowPos(position_, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size_, ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_NoScrollWithMouse |
                                ImGuiWindowFlags_NoResize;

        bool open = is_visible_;
        if (ImGui::Begin("Channel Selector", &open, flags)) {
            render_header(group_name);
            render_channels_list(channels, selected_count, total_count);
            render_footer(can_confirm, is_new_group);
        }
        ImGui::End();

        // Handle window close via X button
        if (!open && callbacks_.on_cancel) {
            callbacks_.on_cancel();
        }
    }

    void ChannelsGroupView::render_header(const std::string& group_name) {
        ImGui::SeparatorText("Configuration Name");
        ImGui::Spacing();

        // Update buffer if groupName changed externally
        if (group_name != name_input_buffer_) {
            strncpy(name_input_buffer_, group_name.c_str(), sizeof(name_input_buffer_));
            name_input_buffer_[sizeof(name_input_buffer_) - 1] = '\0';
        }

        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##name_input", name_input_buffer_, sizeof(name_input_buffer_))) {
            if (callbacks_.on_group_name_changed) {
                callbacks_.on_group_name_changed(name_input_buffer_);
            }
        }

        ImGui::Spacing();
    }

    void ChannelsGroupView::render_channels_list(
        const std::vector<models::Channel>& channels,
        int selected_count,
        int total_count
    ) {
        // Section title with count
        char section_title[64];
        std::snprintf(section_title, sizeof(section_title),
                     "Select Channels (%d / %d)", selected_count, total_count);
        ImGui::SeparatorText(section_title);
        ImGui::Spacing();

        if (channels.empty()) {
            ImGui::TextDisabled("No channels available");
            return;
        }

        // Select All / Clear All buttons
        ImGui::BeginGroup();
        if (ImGui::Button("Select All", ImVec2(100, 0))) {
            if (callbacks_.on_select_all_channels) {
                callbacks_.on_select_all_channels(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All", ImVec2(100, 0))) {
            if (callbacks_.on_select_all_channels) {
                callbacks_.on_select_all_channels(false);
            }
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Scrollable channel list
        ImGui::BeginChild("ChannelsList", ImVec2(0, 250), true);

        for (size_t i = 0; i < channels.size(); ++i) {
            const auto& channel = channels[i];
            ImGui::PushID(channel.get_id().c_str());

            bool was_selected = channel.selected;
            bool is_selected = render_custom_checkbox(channel.name.c_str(), channel.selected);

            if (is_selected != was_selected && callbacks_.on_channel_selection_changed) {
                callbacks_.on_channel_selection_changed(i, is_selected);
            }

            ImGui::PopID();
        }

        ImGui::EndChild();
    }

    void ChannelsGroupView::render_footer(bool can_confirm, bool is_new_group) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Confirm button
        if (!can_confirm) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            if (callbacks_.on_confirm) {
                callbacks_.on_confirm();
            }
        }

        if (!can_confirm) {
            ImGui::EndDisabled();
        }

        // Delete or Cancel button (to the right of Confirm)
        ImGui::SameLine();

        if (is_new_group) {
            // Show Cancel button for new groups
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                if (callbacks_.on_cancel) {
                    callbacks_.on_cancel();
                }
            }
        } else {
            // Show Delete button for existing groups
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                if (callbacks_.on_delete) {
                    callbacks_.on_delete();
                }
            }

            ImGui::PopStyleColor(3);
        }
    }

    // ========================================================================
    // CUSTOM CHECKBOX (from original implementation)
    // ========================================================================

    bool ChannelsGroupView::render_custom_checkbox(const char* label, bool value) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return value;

        ImGuiContext& g = *ImGui::GetCurrentContext();
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        const float square_sz = 20.0f;
        const ImVec2 pos = window->DC.CursorPos;
        const float total_w = square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f);
        const float total_h = ImMax(square_sz, label_size.y);
        const ImRect total_bb(pos, ImVec2(pos.x + total_w, pos.y + total_h));

        ImGui::ItemSize(total_bb, 0.0f);
        if (!ImGui::ItemAdd(total_bb, id))
            return value;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        bool new_value = value;
        if (pressed)
            new_value = !value;

        // Center checkbox vertically
        float check_y_offset = (total_h - square_sz) * 0.5f;
        const ImRect check_bb(ImVec2(pos.x, pos.y + check_y_offset),
                              ImVec2(pos.x + square_sz, pos.y + check_y_offset + square_sz));

        ImGui::RenderNavHighlight(total_bb, id);

        // Colors
        ImU32 bg_col;
        ImU32 border_col;

        if (value) {
            bg_col = ImGui::GetColorU32(ImVec4(0.53f, 0.75f, 0.98f, 1.0f));
            border_col = ImGui::GetColorU32(ImVec4(0.53f, 0.75f, 0.98f, 1.0f));
        } else {
            bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive :
                                       hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
            border_col = ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        }

        // Draw rounded rectangle
        float rounding = 4.0f;
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, bg_col, rounding);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, border_col, rounding, 0, 1.5f);

        // Draw checkmark if checked
        if (value) {
            const float pad = square_sz * 0.25f;
            const float thickness = 2.0f;

            ImVec2 center = ImVec2((check_bb.Min.x + check_bb.Max.x) * 0.5f,
                                  (check_bb.Min.y + check_bb.Max.y) * 0.5f);
            float size = square_sz - pad * 2.0f;

            ImVec2 p1 = ImVec2(center.x - size * 0.35f, center.y);
            ImVec2 p2 = ImVec2(center.x - size * 0.1f, center.y + size * 0.3f);
            ImVec2 p3 = ImVec2(center.x + size * 0.4f, center.y - size * 0.35f);

            ImU32 check_col = ImGui::GetColorU32(ImVec4(0.1f, 0.3f, 0.6f, 1.0f));
            window->DrawList->AddPolyline((const ImVec2[]){p1, p2, p3}, 3, check_col, 0, thickness);
        }

        // Draw label
        if (label_size.x > 0.0f) {
            float label_y_offset = (total_h - label_size.y) * 0.5f;
            ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x,
                                    pos.y + label_y_offset), label);
        }

        return new_value;
    }

} // namespace elda::channels_group