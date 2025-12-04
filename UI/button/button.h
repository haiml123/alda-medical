#pragma once

#include "imgui.h"

#include <functional>
#include <string>

namespace elda::ui
{

/**
 * @brief Style configuration for UI buttons
 */
struct ButtonStyle
{
    ImVec2 size = ImVec2(90, 32);  // Button dimensions
    float rounding = 4.0f;         // Corner rounding

    // Colors
    ImVec4 normalColor = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    ImVec4 hoverColor = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    ImVec4 activeColor = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    ImVec4 disabledColor = ImVec4(0.10f, 0.10f, 0.10f, 0.50f);

    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 textDisabledColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Padding
    ImVec2 padding = ImVec2(8.0f, 4.0f);

    // Font
    ImFont* font = nullptr;  // nullptr = use default font
};

/**
 * @brief Predefined button style presets for common use cases
 */
namespace ButtonPresets
{
// Primary action button (blue)
inline ButtonStyle Primary()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.hoverColor = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    style.activeColor = ImVec4(0.16f, 0.46f, 0.90f, 1.00f);
    return style;
}

// Success/Start button (green)
inline ButtonStyle Success()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.20f, 0.90f, 0.35f, 1.00f);
    style.hoverColor = ImVec4(0.18f, 0.80f, 0.32f, 1.00f);
    style.activeColor = ImVec4(0.16f, 0.70f, 0.30f, 1.00f);
    return style;
}

// Warning/Pause button (orange)
inline ButtonStyle Warning()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.95f, 0.75f, 0.25f, 1.00f);
    style.hoverColor = ImVec4(0.92f, 0.70f, 0.22f, 1.00f);
    style.activeColor = ImVec4(0.88f, 0.65f, 0.20f, 1.00f);
    return style;
}

// Danger/Stop button (red)
inline ButtonStyle Danger()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);
    style.hoverColor = ImVec4(0.85f, 0.28f, 0.25f, 1.00f);
    style.activeColor = ImVec4(0.80f, 0.23f, 0.20f, 1.00f);
    return style;
}

// Secondary/Neutral button (purple/gray)
inline ButtonStyle Secondary()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.60f, 0.40f, 0.80f, 1.00f);
    style.hoverColor = ImVec4(0.65f, 0.45f, 0.85f, 1.00f);
    style.activeColor = ImVec4(0.55f, 0.35f, 0.75f, 1.00f);
    return style;
}

// Dark/Inactive button
inline ButtonStyle Dark()
{
    ButtonStyle style;
    style.normalColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.hoverColor = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    style.activeColor = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    return style;
}
}  // namespace ButtonPresets

/**
 * @brief Reusable UI Button Component
 *
 * A flexible button component with customizable styling, sizing, and behavior.
 * Supports enabled/disabled states, custom callbacks, and icon support.
 */
class Button
{
  public:
    Button(const std::string& label = "Button", const ButtonStyle& style = ButtonPresets::Primary());

    // Core rendering
    bool render();

    // Setters
    void setLabel(const std::string& label)
    {
        label_ = label;
    }
    void setStyle(const ButtonStyle& style)
    {
        style_ = style;
    }
    void setSize(const ImVec2& size)
    {
        style_.size = size;
    }
    void setEnabled(bool enabled)
    {
        enabled_ = enabled;
    }
    void setCallback(std::function<void()> callback)
    {
        callback_ = callback;
    }

    // Optional icon support (render icon before label)
    void setIcon(const char* iconText)
    {
        icon_ = iconText;
    }
    void clearIcon()
    {
        icon_.clear();
    }

    // Tooltip support
    void setTooltip(const std::string& tooltip)
    {
        tooltip_ = tooltip;
    }

    // Getters
    const std::string& getLabel() const
    {
        return label_;
    }
    const ButtonStyle& getStyle() const
    {
        return style_;
    }
    bool isEnabled() const
    {
        return enabled_;
    }
    bool wasClicked() const
    {
        return wasClicked_;
    }

    // Static helper for one-off buttons without creating an object
    static bool
    RenderButton(const std::string& label, const ButtonStyle& style = ButtonPresets::Primary(), bool enabled = true);

  private:
    std::string label_;
    std::string icon_;
    std::string tooltip_;
    ButtonStyle style_;
    bool enabled_;
    bool wasClicked_;
    std::function<void()> callback_;
};

}  // namespace elda::ui