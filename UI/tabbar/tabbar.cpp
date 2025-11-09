#include "tabbar.h"
#include <algorithm>

namespace elda::ui {

TabBar::TabBar()
    : activeTabIndex_(0), hoveredTabIndex_(-1) {
}

TabBar::TabBar(const TabBarStyle& style)
    : activeTabIndex_(0), hoveredTabIndex_(-1), style_(style) {
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
    }
}

void TabBar::clear() {
    tabs_.clear();
    activeTabIndex_ = 0;
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
// RENDERING
// ============================================================================

bool TabBar::render() {
    if (tabs_.empty()) {
        return false;
    }

    bool tabChanged = false;

    // Begin child window for tab bar
    ImGui::BeginChild("##TabBarContainer",
                     ImVec2(0, style_.height),
                     false,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Add top spacing
    ImGui::Spacing();

    // Calculate tab width for auto-sizing
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float totalSpacing = style_.spacing * (tabs_.size() - 1);
    float autoTabWidth = style_.autoSize ?
        (availableWidth - totalSpacing) / tabs_.size() : 0.0f;

    // Render each tab
    for (size_t i = 0; i < tabs_.size(); ++i) {
        const Tab& tab = tabs_[i];
        const bool isActive = (static_cast<int>(i) == activeTabIndex_);

        renderTab(static_cast<int>(i), tab, isActive);

        // Check for click
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (tab.enabled && !isActive) {
                activeTabIndex_ = static_cast<int>(i);
                tabChanged = true;

                if (onTabClick_) {
                    onTabClick_(static_cast<int>(i), tab);
                }
            }
        }

        // Check for right-click
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (tab.enabled && onTabRightClick_) {
                onTabRightClick_(static_cast<int>(i), tab);
            }
        }

        // Check for hover
        if (ImGui::IsItemHovered()) {
            hoveredTabIndex_ = static_cast<int>(i);
            if (onTabHover_) {
                onTabHover_(static_cast<int>(i), tab);
            }
        }

        // Add spacing between tabs (not after last tab)
        if (i < tabs_.size() - 1) {
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(style_.spacing, 0));
            ImGui::SameLine();
        }
    }

    ImGui::EndChild();

    // Render separator line if enabled
    if (style_.showSeparator) {
        ImGui::Separator();
    }

    return tabChanged;
}

void TabBar::renderTab(int index, const Tab& tab, bool isActive) {
    const bool isHovered = (hoveredTabIndex_ == index);
    const bool isDisabled = !tab.enabled;

    // Apply styling
    applyTabStyle(isActive, isHovered, isDisabled);

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

    // Render button
    if (isDisabled) {
        ImGui::BeginDisabled();
    }

    ImGui::Button(buttonId, size);

    if (isDisabled) {
        ImGui::EndDisabled();
    }

    // Restore styling
    restoreStyle();
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
    if (style_.autoSize) {
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float totalSpacing = style_.spacing * (tabs_.size() - 1);
        float tabWidth = (availableWidth - totalSpacing) / tabs_.size();
        return ImVec2(tabWidth, 0);
    }
    return ImVec2(0, 0); // Auto-size by content
}

void TabBar::applyTabStyle(bool isActive, bool isHovered, bool isDisabled) {
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