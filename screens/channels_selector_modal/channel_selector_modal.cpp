#include "channel_selector_modal.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cstdio>

void ChannelSelectorModal::Open(std::vector<Channel>* channels, OnConfirmCallback callback) {
    isOpen = true;
    channelsRef = channels;
    onConfirm = callback;
    inputName.clear();
}

void ChannelSelectorModal::Close() {
    isOpen = false;
    inputName.clear();
}

// Custom checkbox with rounded corners and thick checkmark like the mockup
static bool CustomCheckbox(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = 20.0f; // Fixed smaller size
    const ImVec2 pos = window->DC.CursorPos;
    const float total_w = square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f);
    const float total_h = ImMax(square_sz, label_size.y); // Use max height for proper alignment
    const ImRect total_bb(pos, ImVec2(pos.x + total_w, pos.y + total_h));

    ImGui::ItemSize(total_bb, 0.0f);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        *v = !(*v);

    // Center the checkbox vertically if label is taller
    float check_y_offset = (total_h - square_sz) * 0.5f;
    const ImRect check_bb(ImVec2(pos.x, pos.y + check_y_offset),
                          ImVec2(pos.x + square_sz, pos.y + check_y_offset + square_sz));

    ImGui::RenderNavHighlight(total_bb, id);

    // Colors for the checkbox
    ImU32 bg_col;
    ImU32 border_col;

    if (*v) {
        // When checked: light blue background
        bg_col = ImGui::GetColorU32(ImVec4(0.53f, 0.75f, 0.98f, 1.0f)); // Light blue
        border_col = ImGui::GetColorU32(ImVec4(0.53f, 0.75f, 0.98f, 1.0f)); // Same light blue
    } else {
        // When unchecked: dark background
        bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        border_col = ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); // Gray border
    }

    // Draw rounded rectangle background
    float rounding = 4.0f; // Rounded corners
    window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, bg_col, rounding);

    // Draw border
    window->DrawList->AddRect(check_bb.Min, check_bb.Max, border_col, rounding, 0, 1.5f);

    // Draw thick dark blue checkmark if checked
    if (*v) {
        const float pad = square_sz * 0.25f;
        const float thickness = 2.0f; // Thick checkmark

        // Calculate checkmark points
        ImVec2 center = ImVec2((check_bb.Min.x + check_bb.Max.x) * 0.5f, (check_bb.Min.y + check_bb.Max.y) * 0.5f);
        float size = square_sz - pad * 2.0f;

        // Checkmark path
        ImVec2 p1 = ImVec2(center.x - size * 0.35f, center.y);
        ImVec2 p2 = ImVec2(center.x - size * 0.1f, center.y + size * 0.3f);
        ImVec2 p3 = ImVec2(center.x + size * 0.4f, center.y - size * 0.35f);

        // Dark blue checkmark
        ImU32 check_col = ImGui::GetColorU32(ImVec4(0.1f, 0.3f, 0.6f, 1.0f)); // Dark blue

        // Draw checkmark with thickness
        window->DrawList->AddPolyline((const ImVec2[]){p1, p2, p3}, 3, check_col, 0, thickness);
    }

    // Draw label - center it vertically with the checkbox
    if (label_size.x > 0.0f) {
        float label_y_offset = (total_h - label_size.y) * 0.5f;
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, pos.y + label_y_offset), label);
    }

    return pressed;
}

void ChannelSelectorModal::DrawChannelsList() {
    if (!channelsRef || channelsRef->empty()) {
        ImGui::TextDisabled("No channels available");
        return;
    }

    // Add Select All / Clear All buttons
    ImGui::BeginGroup();
    if (ImGui::Button("Select All", ImVec2(100, 0))) {
        for (auto& ch : *channelsRef) {
            ch.selected = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All", ImVec2(100, 0))) {
        for (auto& ch : *channelsRef) {
            ch.selected = false;
        }
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Scrollable channel list with reduced height to make room for Confirm button
    ImGui::BeginChild("ChannelsList", ImVec2(0, 250), true);

    for (auto& channel : *channelsRef) {
        ImGui::PushID(channel.id.c_str());

        // Use custom checkbox with rounded corners
        CustomCheckbox(channel.name.c_str(), &channel.selected);

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void ChannelSelectorModal::Draw(ImVec2 buttonPos, ImVec2 buttonSize) {
    if (!isOpen) return;

    // Position popup below and aligned with the button
    ImVec2 popupPos = ImVec2(buttonPos.x, buttonPos.y + buttonSize.y + 2.0f);
    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 550), ImGuiCond_Always); // Increased height to 550

    // No scrollbar, no resize flags
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoResize; // Disable resizing

    if (ImGui::Begin("Channel Selector", &isOpen, window_flags)) {

        // Name input section
        ImGui::SeparatorText("Configuration Name");
        ImGui::Spacing();

        char buffer[256];
        strncpy(buffer, inputName.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##name_input", buffer, sizeof(buffer))) {
            inputName = buffer;
        }

        ImGui::Spacing();

        // Count selected channels
        int selectedCount = 0;
        if (channelsRef) {
            selectedCount = std::count_if(channelsRef->begin(), channelsRef->end(),
                                         [](const Channel& ch) { return ch.selected; });
        }

        // Channel selection section with count
        char sectionTitle[64];
        std::snprintf(sectionTitle, sizeof(sectionTitle), "Select Channels (%d / %d)",
                     selectedCount, channelsRef ? (int)channelsRef->size() : 0);
        ImGui::SeparatorText(sectionTitle);
        ImGui::Spacing();

        DrawChannelsList();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Only Confirm button
        bool canConfirm = !inputName.empty() && selectedCount > 0;

        if (!canConfirm) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            if (onConfirm && channelsRef) {
                onConfirm(inputName, *channelsRef);
            }
            Close();
        }

        if (!canConfirm) {
            ImGui::EndDisabled();
        }
    }

    // If window was closed via X button
    if (!isOpen) {
        Close();
    }

    ImGui::End();
}