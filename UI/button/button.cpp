#include "button.h"

namespace elda::ui {

Button::Button(const std::string& label, const ButtonStyle& style)
    : label_(label)
    , style_(style)
    , enabled_(true)
    , wasClicked_(false)
    , callback_(nullptr) {
}

bool Button::render() {
    wasClicked_ = false;

    // Disable if needed
    if (!enabled_) {
        ImGui::BeginDisabled();
    }

    // Apply custom font if specified
    if (style_.font) {
        ImGui::PushFont(style_.font);
    }

    // Apply button styling
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style_.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style_.padding);

    ImGui::PushStyleColor(ImGuiCol_Button, enabled_ ? style_.normalColor : style_.disabledColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, enabled_ ? style_.hoverColor : style_.disabledColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, enabled_ ? style_.activeColor : style_.disabledColor);
    ImGui::PushStyleColor(ImGuiCol_Text, enabled_ ? style_.textColor : style_.textDisabledColor);

    // Build button label with optional icon
    std::string displayLabel = label_;
    if (!icon_.empty()) {
        displayLabel = icon_ + " " + label_;
    }

    // Render button
    bool clicked = ImGui::Button(displayLabel.c_str(), style_.size);

    // Tooltip if hovering and tooltip is set
    if (!tooltip_.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", tooltip_.c_str());
        ImGui::EndTooltip();
    }

    // Pop styles
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    if (style_.font) {
        ImGui::PopFont();
    }

    if (!enabled_) {
        ImGui::EndDisabled();
    }

    // Handle click
    if (clicked && enabled_) {
        wasClicked_ = true;
        if (callback_) {
            callback_();
        }
    }

    return wasClicked_;
}

bool Button::RenderButton(const std::string& label, const ButtonStyle& style, bool enabled) {
    Button btn(label, style);
    btn.setEnabled(enabled);
    return btn.render();
}

}