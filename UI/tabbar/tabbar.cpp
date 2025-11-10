#include "tabbar.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace elda::ui {

TabBar::TabBar()
    : activeTabIndex_(0), hoveredTabIndex_(-1) {
}

TabBar::TabBar(const TabBarStyle& style)
    : activeTabIndex_(0), style_(style), hoveredTabIndex_(-1) {
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void TabBar::setTabs(const std::vector<Tab>& tabs) {
    tabs_ = tabs;
    // Clamp active index to valid range
    if (activeTabIndex_ >= static_cast<int>(tabs_.size())) {
        activeTabIndex_ = std::max(0, static_cast<int>(tabs_.size()) - 1);
    }
    // Clear bounds since tabs changed
    tabBounds_.clear();
    // Clear bounds pointers in tabs
    for (auto& tab : tabs_) {
        tab.bounds = nullptr;
    }
}

void TabBar::setActiveTab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        activeTabIndex_ = index;
    }
}

void TabBar::setStyle(const TabBarStyle& style) {
    style_ = style;
}

void TabBar::addTab(const Tab& tab) {
    tabs_.push_back(tab);
}

void TabBar::removeTab(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        tabs_.erase(tabs_.begin() + index);

        // Adjust active tab if needed
        if (activeTabIndex_ >= static_cast<int>(tabs_.size())) {
            activeTabIndex_ = std::max(0, static_cast<int>(tabs_.size()) - 1);
        }

        // Clear bounds since tabs changed (will be recalculated on next render)
        tabBounds_.clear();
        for (auto& tab : tabs_) {
            tab.bounds = nullptr;
        }
    }
}

void TabBar::clear() {
    tabs_.clear();
    activeTabIndex_ = 0;
    tabBounds_.clear();
}

void TabBar::setBadge(int tabIndex, int badge) {
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabs_.size())) {
        tabs_[tabIndex].badge = badge;
    }
}

void TabBar::setTabEnabled(int tabIndex, bool enabled) {
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabs_.size())) {
        tabs_[tabIndex].enabled = enabled;
    }
}

const Tab* TabBar::getTab(int index) const {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        return &tabs_[index];
    }
    return nullptr;
}

// ============================================================================
// TAB BOUNDS QUERIES
// ============================================================================

const TabBounds* TabBar::getTabBounds(int index) const {
    if (index >= 0 && index < static_cast<int>(tabBounds_.size())) {
        return &tabBounds_[index];
    }
    return nullptr;
}

int TabBar::getTabAtPosition(float x, float y) const {
    for (int i = 0; i < static_cast<int>(tabBounds_.size()); ++i) {
        if (tabBounds_[i].contains(x, y)) {
            return i;
        }
    }
    return -1;
}

// ============================================================================
// RENDERING
// ============================================================================

bool TabBar::render() {
    if (tabs_.empty()) {
        return false;
    }

    // Clear previous bounds pointers
    for (auto& tab : tabs_) {
        tab.bounds = nullptr;
    }

    // Clear previous bounds
    tabBounds_.clear();
    tabBounds_.reserve(tabs_.size());

    bool tabChanged = false;

    // Get draw list for custom rendering
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // Draw background if enabled
    if (style_.showBackground) {
        ImVec2 bgMin = cursorPos;
        ImVec2 bgMax = ImVec2(cursorPos.x + ImGui::GetContentRegionAvail().x,
                              cursorPos.y + style_.height);
        drawList->AddRectFilled(bgMin, bgMax,
                               ImGui::GetColorU32(style_.backgroundColor));
    }

    // Begin child window for tab bar
    ImGui::BeginChild("##TabBarContainer",
                     ImVec2(0, style_.height),
                     false,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Push item spacing for browser-style tabs (controlled by style.spacing)
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style_.spacing, 0));

    // Add top padding
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style_.tabBarPadding);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style_.tabBarPadding);

    // Render each tab
    for (size_t i = 0; i < tabs_.size(); ++i) {
        const Tab& tab = tabs_[i];
        const bool isActive = (static_cast<int>(i) == activeTabIndex_);

        renderTab(static_cast<int>(i), tab, isActive);

        // IMPORTANT: Assign bounds pointer immediately after renderTab stores it
        // This ensures tab.bounds is valid when callbacks fire
        if (i < tabBounds_.size()) {
            tabs_[i].bounds = &tabBounds_[i];
        }

        // Check for click
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (tab.enabled && !isActive) {
                activeTabIndex_ = static_cast<int>(i);
                tabChanged = true;

                if (onTabClick_) {
                    onTabClick_(static_cast<int>(i), tabs_[i]);  // Use tabs_[i] to get updated bounds
                }
            }
        }

        // Check for double-click
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
            if (tab.enabled && onTabDoubleClick_) {
                onTabDoubleClick_(static_cast<int>(i), tabs_[i]);  // Use tabs_[i] to get updated bounds
            }
        }

        // Check for right-click
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (tab.enabled && onTabRightClick_) {
                onTabRightClick_(static_cast<int>(i), tabs_[i]);  // Use tabs_[i] to get updated bounds
            }
        }

        // Check for hover
        if (ImGui::IsItemHovered()) {
            hoveredTabIndex_ = static_cast<int>(i);
            if (onTabHover_) {
                onTabHover_(static_cast<int>(i), tabs_[i]);  // Use tabs_[i] to get updated bounds
            }
        }

        // Position next tab on same line (spacing controlled by ItemSpacing above)
        if (i < tabs_.size() - 1) {
            ImGui::SameLine();  // Use default spacing (from ItemSpacing style var)
        }
    }

    // Render add button if callback is set or showAddButton is enabled
    if (onAddTab_ || style_.showAddButton) {
        ImGui::SameLine();
        renderAddButton();
    }

    // Restore item spacing
    ImGui::PopStyleVar();

    ImGui::EndChild();

    // Render separator line if enabled
    // Draw it at the bottom edge of the child window (no gap)
    if (style_.showSeparator) {
        ImVec2 separatorStart = ImVec2(cursorPos.x, cursorPos.y + style_.height - 1);
        ImVec2 separatorEnd = ImVec2(cursorPos.x + ImGui::GetContentRegionAvail().x,
                                     cursorPos.y + style_.height - 1);
        drawList->AddLine(separatorStart, separatorEnd,
                         ImGui::GetColorU32(style_.separatorColor),
                         style_.separatorHeight);
    }

    return tabChanged;
}

void TabBar::renderTab(int index, const Tab& tab, bool isActive) {
    const bool isHovered = (hoveredTabIndex_ == index);
    const bool isDisabled = !tab.enabled;

    // Build label with optional badge
    char label[256];
    if (style_.showBadges && tab.badge >= 0) {
        std::snprintf(label, sizeof(label), "%s (%d)", tab.label.c_str(), tab.badge);
    } else {
        std::snprintf(label, sizeof(label), "%s", tab.label.c_str());
    }

    // Create unique ID for button
    char buttonId[300];
    std::snprintf(buttonId, sizeof(buttonId), "%s##tab_%d", label, index);

    // Calculate size
    ImVec2 size = calculateTabSize(tab);

    // Get positions for custom drawing
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 textSize = ImGui::CalcTextSize(label);

    // Calculate actual tab size
    ImVec2 actualSize = size;
    if (actualSize.x == 0) {
        actualSize.x = textSize.x + style_.buttonPaddingX * 2;
    }
    if (actualSize.y == 0) {
        // Tab height should fill the container minus the top padding
        actualSize.y = style_.height - style_.tabBarPadding;
    }

    // Store tab bounds
    tabBounds_.emplace_back(cursorPos.x, cursorPos.y, actualSize.x, actualSize.y);

    // Get draw list
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Determine colors
    ImVec4 bgColor;
    if (isDisabled) {
        bgColor = style_.disabledColor;
    } else if (isActive) {
        bgColor = style_.activeColor;
    } else if (isHovered) {
        bgColor = style_.hoverColor;
    } else {
        bgColor = style_.inactiveColor;
    }

    ImU32 bgColorU32 = ImGui::GetColorU32(bgColor);
    ImU32 borderColorU32 = ImGui::GetColorU32(style_.borderColor);

    // Draw browser-style tab with custom path
    if (style_.browserStyle) {
        // ImGui coordinate system: (0,0) is top-left, Y increases downward, X increases rightward
        // We want: rounded top corners, square bottom corners

        float rounding = style_.rounding;

        // Define the four corners of the tab
        ImVec2 topLeft = cursorPos;
        ImVec2 topRight = ImVec2(cursorPos.x + actualSize.x, cursorPos.y);
        ImVec2 bottomRight = ImVec2(cursorPos.x + actualSize.x, cursorPos.y + actualSize.y);
        ImVec2 bottomLeft = ImVec2(cursorPos.x, cursorPos.y + actualSize.y);

        // Build a custom path for browser-style tabs
        // Start at bottom-left, go up to top-left, arc around top-left corner,
        // go across to top-right, arc around top-right corner, go down to bottom-right,
        // then straight back to bottom-left.

        drawList->PathClear();

        // Bottom-left corner (square)
        drawList->PathLineTo(bottomLeft);

        // Left edge going up (leaving room for top-left arc)
        drawList->PathLineTo(ImVec2(topLeft.x, topLeft.y + rounding));

        // Top-left arc (π to 1.5π, or 180° to 270°)
        drawList->PathArcTo(
            ImVec2(topLeft.x + rounding, topLeft.y + rounding),  // center
            rounding,                                            // radius
            M_PI,                                                // start angle (left)
            M_PI * 1.5f,                                         // end angle (up)
            8                                                    // segments
        );

        // Top edge (from left arc to right arc)
        drawList->PathLineTo(ImVec2(topRight.x - rounding, topRight.y));

        // Top-right arc (1.5π to 2π, or 270° to 360°)
        drawList->PathArcTo(
            ImVec2(topRight.x - rounding, topRight.y + rounding), // center
            rounding,                                             // radius
            M_PI * 1.5f,                                          // start angle (up)
            M_PI * 2.0f,                                          // end angle (right)
            8                                                     // segments
        );

        // Right edge going down (square bottom)
        drawList->PathLineTo(bottomRight);

        // Fill the path
        drawList->PathFillConvex(bgColorU32);

        // Draw borders if enabled (using individual line segments + arcs)
        if (style_.showBorders) {
            // Left border
            drawList->AddLine(
                bottomLeft,
                ImVec2(topLeft.x, topLeft.y + rounding),
                borderColorU32, style_.borderThickness
            );

            // Top-left arc
            drawList->PathClear();
            drawList->PathArcTo(
                ImVec2(topLeft.x + rounding, topLeft.y + rounding),
                rounding, M_PI, M_PI * 1.5f, 8
            );
            drawList->PathStroke(borderColorU32, false, style_.borderThickness);

            // Top border
            drawList->AddLine(
                ImVec2(topLeft.x + rounding, topLeft.y),
                ImVec2(topRight.x - rounding, topRight.y),
                borderColorU32, style_.borderThickness
            );

            // Top-right arc
            drawList->PathClear();
            drawList->PathArcTo(
                ImVec2(topRight.x - rounding, topRight.y + rounding),
                rounding, M_PI * 1.5f, M_PI * 2.0f, 8
            );
            drawList->PathStroke(borderColorU32, false, style_.borderThickness);

            // Right border
            drawList->AddLine(
                ImVec2(topRight.x, topRight.y + rounding),
                bottomRight,
                borderColorU32, style_.borderThickness
            );

            // Bottom border
            drawList->AddLine(
                bottomLeft, bottomRight,
                borderColorU32, style_.borderThickness
            );
        }
    } else {
        // Regular rounded rectangle
        ImVec2 rectMin = cursorPos;
        ImVec2 rectMax = ImVec2(cursorPos.x + actualSize.x, cursorPos.y + actualSize.y);

        drawList->AddRectFilled(rectMin, rectMax, bgColorU32, style_.rounding);

        if (style_.showBorders) {
            drawList->AddRect(rectMin, rectMax, borderColorU32,
                            style_.rounding, 0, style_.borderThickness);
        }
    }

    // Create invisible button for interaction
    ImGui::SetCursorScreenPos(cursorPos);

    if (isDisabled) {
        ImGui::BeginDisabled();
    }

    // Invisible button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::Button(buttonId, actualSize);
    ImGui::PopStyleColor(3);

    // Draw text on top
    ImVec2 textPos = ImVec2(
        cursorPos.x + (actualSize.x - textSize.x) * 0.5f,
        cursorPos.y + (actualSize.y - textSize.y) * 0.5f
    );

    ImU32 textColor = isDisabled ?
        ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)) :
        ImGui::GetColorU32(ImGuiCol_Text);

    drawList->AddText(textPos, textColor, label);

    if (isDisabled) {
        ImGui::EndDisabled();
    }

    // Move cursor for next element
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + actualSize.x, cursorPos.y));
}

void TabBar::renderAddButton() {
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // Add button size - smaller and square
    float buttonSize = style_.height - style_.tabBarPadding;
    ImVec2 size = ImVec2(buttonSize, buttonSize);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Define corners
    ImVec2 topLeft = cursorPos;
    ImVec2 topRight = ImVec2(cursorPos.x + size.x, cursorPos.y);
    ImVec2 bottomRight = ImVec2(cursorPos.x + size.x, cursorPos.y + size.y);
    ImVec2 bottomLeft = ImVec2(cursorPos.x, cursorPos.y + size.y);

    // Colors
    ImVec4 bgColor = style_.inactiveColor;
    ImU32 bgColorU32 = ImGui::GetColorU32(bgColor);
    ImU32 borderColorU32 = ImGui::GetColorU32(style_.borderColor);

    float rounding = style_.rounding;

    // Draw browser-style add button (same style as tabs)
    if (style_.browserStyle) {
        // Build path with rounded top corners
        drawList->PathLineTo(bottomLeft);
        drawList->PathLineTo(ImVec2(topLeft.x, topLeft.y + rounding));
        drawList->PathArcTo(
            ImVec2(topLeft.x + rounding, topLeft.y + rounding),
            rounding, M_PI, M_PI * 1.5f, 8
        );
        drawList->PathLineTo(ImVec2(topRight.x - rounding, topRight.y));
        drawList->PathArcTo(
            ImVec2(topRight.x - rounding, topRight.y + rounding),
            rounding, M_PI * 1.5f, M_PI * 2.0f, 8
        );
        drawList->PathLineTo(bottomRight);
        drawList->PathFillConvex(bgColorU32);

        // Draw borders
        if (style_.showBorders) {
            // Top-left arc
            drawList->PathClear();
            drawList->PathArcTo(
                ImVec2(topLeft.x + rounding, topLeft.y + rounding),
                rounding, M_PI, M_PI * 1.5f, 8
            );
            drawList->PathStroke(borderColorU32, false, style_.borderThickness);

            // Top border
            drawList->AddLine(
                ImVec2(topLeft.x + rounding, topLeft.y),
                ImVec2(topRight.x - rounding, topRight.y),
                borderColorU32, style_.borderThickness
            );

            // Top-right arc
            drawList->PathClear();
            drawList->PathArcTo(
                ImVec2(topRight.x - rounding, topRight.y + rounding),
                rounding, M_PI * 1.5f, M_PI * 2.0f, 8
            );
            drawList->PathStroke(borderColorU32, false, style_.borderThickness);

            // Right border
            drawList->AddLine(
                ImVec2(topRight.x, topRight.y + rounding),
                bottomRight,
                borderColorU32, style_.borderThickness
            );

            // Bottom border
            drawList->AddLine(
                bottomLeft, bottomRight,
                borderColorU32, style_.borderThickness
            );
        }
    } else {
        // Regular rounded rectangle
        drawList->AddRectFilled(topLeft, bottomRight, bgColorU32, rounding);
        if (style_.showBorders) {
            drawList->AddRect(topLeft, bottomRight, borderColorU32,
                            rounding, 0, style_.borderThickness);
        }
    }

    // Invisible button for interaction
    ImGui::SetCursorScreenPos(cursorPos);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 0.9f, 0.3f));

    if (ImGui::Button("##AddTab", size)) {
        if (onAddTab_) {
            onAddTab_();
        }
    }

    ImGui::PopStyleColor(3);

    // Draw "+" symbol
    ImVec2 center = ImVec2(cursorPos.x + size.x * 0.5f, cursorPos.y + size.y * 0.5f);
    float plusSize = size.x * style_.addButtonIconSize;  // Use configurable size
    ImU32 plusColor = ImGui::GetColorU32(ImGuiCol_Text);
    float thickness = 2.0f;

    // Horizontal line
    drawList->AddLine(
        ImVec2(center.x - plusSize, center.y),
        ImVec2(center.x + plusSize, center.y),
        plusColor, thickness
    );

    // Vertical line
    drawList->AddLine(
        ImVec2(center.x, center.y - plusSize),
        ImVec2(center.x, center.y + plusSize),
        plusColor, thickness
    );

    // Move cursor for next element
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + size.x, cursorPos.y));
}

void TabBar::renderBadge(int badgeCount) {
    // This is a helper for potential future badge-as-overlay rendering
    // Currently badges are rendered inline with the label
    if (badgeCount < 0) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMax();
    pos.x -= 20.0f;
    pos.y += 4.0f;

    char badge[16];
    std::snprintf(badge, sizeof(badge), "%d", badgeCount);

    ImVec2 textSize = ImGui::CalcTextSize(badge);
    float radius = std::max(textSize.x, textSize.y) * 0.5f + 4.0f;

    drawList->AddCircleFilled(pos, radius, ImGui::GetColorU32(style_.badgeColor));
    drawList->AddText(
        ImVec2(pos.x - textSize.x * 0.5f, pos.y - textSize.y * 0.5f),
        ImGui::GetColorU32(style_.badgeTextColor),
        badge
    );
}

ImVec2 TabBar::calculateTabSize(const Tab& tab) const {
    // Build label for size calculation
    char label[256];
    if (style_.showBadges && tab.badge >= 0) {
        std::snprintf(label, sizeof(label), "%s (%d)", tab.label.c_str(), tab.badge);
    } else {
        std::snprintf(label, sizeof(label), "%s", tab.label.c_str());
    }

    ImVec2 textSize = ImGui::CalcTextSize(label);

    if (style_.autoSize) {
        float availableWidth = ImGui::GetContentRegionAvail().x - (style_.tabBarPadding * 2);
        float totalSpacing = style_.spacing * (tabs_.size() - 1);
        float tabWidth = (availableWidth - totalSpacing) / tabs_.size();
        return ImVec2(tabWidth, textSize.y + style_.buttonPaddingY * 2);
    }

    // Auto-size by content
    return ImVec2(0, 0);
}

void TabBar::applyTabStyle(bool isActive, bool /*isHovered*/, bool isDisabled) {
    // Push style variables
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style_.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                       ImVec2(style_.buttonPaddingX, style_.buttonPaddingY));

    // Push colors based on state
    ImVec4 bgColor;
    if (isDisabled) {
        bgColor = style_.disabledColor;
    } else if (isActive) {
        bgColor = style_.activeColor;
    } else {
        bgColor = style_.inactiveColor;
    }

    ImVec4 hoverColor = isActive ? style_.activeColor : style_.hoverColor;

    ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, hoverColor);
}

void TabBar::restoreStyle() {
    ImGui::PopStyleColor(3);  // Button colors
    ImGui::PopStyleVar(2);    // Frame rounding and padding
}

} // namespace elda::ui