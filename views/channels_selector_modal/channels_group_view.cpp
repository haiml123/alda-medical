#include "channels_group_view.h"
#include "imgui_internal.h"
#include <cstring>
#include <cstdio>

namespace elda::channels_group {

    ChannelsGroupView::ChannelsGroupView()
        : isVisible_(false)
        , position_(0, 0)
        , size_(300, 550) {
        nameInputBuffer_[0] = '\0';
    }

    // ========================================================================
    // RENDERING
    // ========================================================================

    void ChannelsGroupView::Render(
        const std::string& groupName,
        const std::vector<models::Channel>& channels,
        int selectedCount,
        int totalCount,
        bool canConfirm
    ) {
        if (!isVisible_) return;

        ImGui::SetNextWindowPos(position_, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size_, ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_NoScrollWithMouse |
                                ImGuiWindowFlags_NoResize;

        bool open = isVisible_;
        if (ImGui::Begin("Channel Selector", &open, flags)) {
            RenderHeader(groupName);
            RenderChannelsList(channels, selectedCount, totalCount);
            RenderFooter(canConfirm);
        }
        ImGui::End();

        // Handle window close via X button
        if (!open && callbacks_.onCancel) {
            callbacks_.onCancel();
        }
    }

    void ChannelsGroupView::RenderHeader(const std::string& groupName) {
        ImGui::SeparatorText("Configuration Name");
        ImGui::Spacing();

        // Update buffer if groupName changed externally
        if (groupName != nameInputBuffer_) {
            strncpy(nameInputBuffer_, groupName.c_str(), sizeof(nameInputBuffer_));
            nameInputBuffer_[sizeof(nameInputBuffer_) - 1] = '\0';
        }

        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##name_input", nameInputBuffer_, sizeof(nameInputBuffer_))) {
            if (callbacks_.onGroupNameChanged) {
                callbacks_.onGroupNameChanged(nameInputBuffer_);
            }
        }

        ImGui::Spacing();
    }

    void ChannelsGroupView::RenderChannelsList(
        const std::vector<models::Channel>& channels,
        int selectedCount,
        int totalCount
    ) {
        // Section title with count
        char sectionTitle[64];
        std::snprintf(sectionTitle, sizeof(sectionTitle),
                     "Select Channels (%d / %d)", selectedCount, totalCount);
        ImGui::SeparatorText(sectionTitle);
        ImGui::Spacing();

        if (channels.empty()) {
            ImGui::TextDisabled("No channels available");
            return;
        }

        // Select All / Clear All buttons
        ImGui::BeginGroup();
        if (ImGui::Button("Select All", ImVec2(100, 0))) {
            if (callbacks_.onSelectAllChannels) {
                callbacks_.onSelectAllChannels(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All", ImVec2(100, 0))) {
            if (callbacks_.onSelectAllChannels) {
                callbacks_.onSelectAllChannels(false);
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
            ImGui::PushID(channel.id.c_str());

            bool wasSelected = channel.selected;
            bool isSelected = RenderCustomCheckbox(channel.name.c_str(), channel.selected);

            if (isSelected != wasSelected && callbacks_.onChannelSelectionChanged) {
                callbacks_.onChannelSelectionChanged(i, isSelected);
            }

            ImGui::PopID();
        }

        ImGui::EndChild();
    }

    void ChannelsGroupView::RenderFooter(bool canConfirm) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (!canConfirm) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            if (callbacks_.onConfirm) {
                callbacks_.onConfirm();
            }
        }

        if (!canConfirm) {
            ImGui::EndDisabled();
        }
    }

    // ========================================================================
    // CUSTOM CHECKBOX (from original implementation)
    // ========================================================================

    bool ChannelsGroupView::RenderCustomCheckbox(const char* label, bool value) {
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
        bool newValue = value;
        if (pressed)
            newValue = !value;

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

        return newValue;
    }

} // namespace elda::channels_group