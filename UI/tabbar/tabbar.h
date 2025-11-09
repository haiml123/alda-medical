#ifndef ELDA_UI_TABBAR_H
#define ELDA_UI_TABBAR_H

#include <string>
#include <vector>
#include <functional>
#include "imgui.h"

namespace elda::ui {

    /**
     * Represents a single tab with label and optional metadata
     */
    struct Tab {
        std::string label;           // Display text (e.g., "Frontal" or "Group 1")
        std::string id;              // Unique identifier (optional, defaults to label)
        int badge = -1;              // Badge count to show (e.g., channel count), -1 = no badge
        void* userData = nullptr;    // Optional user data pointer
        bool enabled = true;         // Whether tab is clickable

        Tab() = default;

        explicit Tab(const std::string& label_)
            : label(label_), id(label_) {}

        Tab(const std::string& label_, int badge_)
            : label(label_), id(label_), badge(badge_) {}

        Tab(const std::string& label_, const std::string& id_)
            : label(label_), id(id_) {}

        Tab(const std::string& label_, int badge_, void* userData_)
            : label(label_), id(label_), badge(badge_), userData(userData_) {}
    };

    /**
     * Style configuration for TabBar
     */
    struct TabBarStyle {
        // Colors
        ImVec4 activeColor      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);  // Active tab
        ImVec4 inactiveColor    = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);  // Inactive tab
        ImVec4 hoverColor       = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);  // Hover state
        ImVec4 disabledColor    = ImVec4(0.10f, 0.10f, 0.10f, 0.50f);  // Disabled tab
        ImVec4 badgeColor       = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);  // Badge background
        ImVec4 badgeTextColor   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // Badge text

        // Sizing
        float height            = 40.0f;     // Tab bar height
        float buttonPaddingX    = 16.0f;     // Horizontal padding inside tab
        float buttonPaddingY    = 8.0f;      // Vertical padding inside tab
        float spacing           = 8.0f;      // Space between tabs
        float rounding          = 4.0f;      // Corner rounding
        float badgeOffsetX      = 4.0f;      // Badge offset from label
        float separatorHeight   = 1.0f;      // Height of separator line below tabs

        // Layout
        bool showSeparator      = true;      // Show separator line below tabs
        bool autoSize           = false;     // Auto-size tabs to fill width
        bool showBadges         = true;      // Show badge counts

        TabBarStyle() = default;
    };

    /**
     * Callback types for TabBar events
     */
    using TabClickCallback = std::function<void(int tabIndex, const Tab& tab)>;
    using TabHoverCallback = std::function<void(int tabIndex, const Tab& tab)>;
    using TabRightClickCallback = std::function<void(int tabIndex, const Tab& tab)>;

    /**
     * Reusable TabBar Component
     *
     * A horizontal tab bar with customizable styling and callbacks.
     * Handles active state, hover effects, badges, and click events.
     *
     * Usage:
     *   TabBar tabBar;
     *   tabBar.setTabs({{"Tab 1", 5}, {"Tab 2", 8}, {"Tab 3", 3}});
     *   tabBar.setActiveTab(0);
     *   tabBar.setOnTabClick([](int idx, const Tab& tab) {
     *       printf("Clicked tab %d\n", idx);
     *   });
     *   tabBar.render();
     */
    class TabBar {
    public:
        TabBar();
        explicit TabBar(const TabBarStyle& style);
        ~TabBar() = default;

        // ========================================================================
        // CONFIGURATION
        // ========================================================================

        /**
         * Set the list of tabs to display
         */
        void setTabs(const std::vector<Tab>& tabs);

        /**
         * Set the active tab index
         */
        void setActiveTab(int index);

        /**
         * Get current active tab index
         */
        int getActiveTab() const { return activeTabIndex_; }

        /**
         * Set custom style
         */
        void setStyle(const TabBarStyle& style);

        /**
         * Get current style (for modification)
         */
        TabBarStyle& getStyle() { return style_; }

        /**
         * Add a single tab
         */
        void addTab(const Tab& tab);

        /**
         * Remove a tab by index
         */
        void removeTab(int index);

        /**
         * Clear all tabs
         */
        void clear();

        /**
         * Update badge count for a specific tab
         */
        void setBadge(int tabIndex, int badge);

        /**
         * Enable/disable a tab
         */
        void setTabEnabled(int tabIndex, bool enabled);

        // ========================================================================
        // CALLBACKS
        // ========================================================================

        /**
         * Set callback for when a tab is clicked
         */
        void setOnTabClick(TabClickCallback callback) { onTabClick_ = callback; }

        /**
         * Set callback for when a tab is hovered
         */
        void setOnTabHover(TabHoverCallback callback) { onTabHover_ = callback; }

        /**
         * Set callback for when a tab is right-clicked
         */
        void setOnTabRightClick(TabRightClickCallback callback) { onTabRightClick_ = callback; }

        // ========================================================================
        // RENDERING
        // ========================================================================

        /**
         * Render the tab bar (call this in your ImGui render loop)
         * Returns true if a tab was clicked (and active tab changed)
         */
        bool render();

        /**
         * Get the number of tabs
         */
        size_t getTabCount() const { return tabs_.size(); }

        /**
         * Check if tab bar has any tabs
         */
        bool isEmpty() const { return tabs_.empty(); }

        /**
         * Get tab by index
         */
        const Tab* getTab(int index) const;

    private:
        std::vector<Tab> tabs_;
        int activeTabIndex_;
        TabBarStyle style_;

        // Callbacks
        TabClickCallback onTabClick_;
        TabHoverCallback onTabHover_;
        TabRightClickCallback onTabRightClick_;

        // Internal state
        int hoveredTabIndex_;  // Currently hovered tab (-1 = none)

        // Helper methods
        void renderTab(int index, const Tab& tab, bool isActive);
        void renderBadge(int badgeCount);
        ImVec2 calculateTabSize(const Tab& tab) const;
        void applyTabStyle(bool isActive, bool isHovered, bool isDisabled);
        void restoreStyle();
    };

} // namespace elda::ui

#endif