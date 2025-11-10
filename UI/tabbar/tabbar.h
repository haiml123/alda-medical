#ifndef ELDA_UI_TABBAR_H
#define ELDA_UI_TABBAR_H

#include <string>
#include <vector>
#include <functional>
#include "imgui.h"

namespace elda::ui {

    // Forward declare TabBounds
    struct TabBounds;

    /**
     * Represents a single tab with label and optional metadata
     */
    struct Tab {
        std::string label;           // Display text (e.g., "Frontal" or "Group 1")
        std::string id;              // Unique identifier (optional, defaults to label)
        int badge = -1;              // Badge count to show (e.g., channel count), -1 = no badge
        void* userData = nullptr;    // Optional user data pointer
        bool enabled = true;         // Whether tab is clickable
        TabBounds* bounds = nullptr; // Bounds of the tab (updated after render)

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
     * Stores the rendered bounds of a tab
     */
    struct TabBounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        TabBounds() = default;
        TabBounds(float x_, float y_, float w_, float h_)
            : x(x_), y(y_), width(w_), height(h_) {}

        // Check if a point is inside this tab
        bool contains(float px, float py) const {
            return px >= x && px <= (x + width) && py >= y && py <= (y + height);
        }

        // Get center point
        ImVec2 center() const {
            return ImVec2(x + width * 0.5f, y + height * 0.5f);
        }

        // Get min/max corners
        ImVec2 min() const {
            return ImVec2(x, y);
        }

        ImVec2 max() const {
            return ImVec2(x + width, y + height);
        }
    };

    /**
     * Style configuration for TabBar
     */
    struct TabBarStyle {
        // Colors - Browser style theme
        ImVec4 activeColor      = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // Active tab (darker)
        ImVec4 inactiveColor    = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);  // Inactive tab
        ImVec4 hoverColor       = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);  // Hover state
        ImVec4 disabledColor    = ImVec4(0.10f, 0.10f, 0.10f, 0.50f);  // Disabled tab
        ImVec4 badgeColor       = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);  // Badge background
        ImVec4 badgeTextColor   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // Badge text
        ImVec4 borderColor      = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);  // Tab border
        ImVec4 backgroundColor  = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // Tab bar background
        ImVec4 separatorColor   = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);  // Bottom separator

        // Sizing
        float height            = 44.0f;     // Tab bar height (includes padding)
        float buttonPaddingX    = 20.0f;     // Horizontal padding inside tab
        float buttonPaddingY    = 10.0f;     // Vertical padding inside tab
        float spacing           = 1.2f;      // Space between tabs (browser-style tight spacing)
        float rounding          = 6.0f;      // Corner rounding (top corners only)
        float badgeOffsetX      = 8.0f;      // Badge offset from label
        float separatorHeight   = 1.0f;      // Height of separator line below tabs
        float borderThickness   = 1.0f;      // Border thickness around tabs
        float tabBarPadding     = 4.0f;      // Padding around tab bar area
        float addButtonIconSize = 0.3f;      // Size of + icon relative to button (0.1-0.5)

        // Layout
        bool showSeparator      = true;      // Show separator line below tabs
        bool autoSize           = false;     // Auto-size tabs to fill width
        bool showBadges         = true;      // Show badge counts
        bool browserStyle       = true;      // Use browser-style (rounded top, square bottom)
        bool showBorders        = true;      // Show borders around tabs
        bool showBackground     = true;      // Show tab bar background
        bool showAddButton      = false;     // Show add (+) button after tabs

        TabBarStyle() = default;
    };

    /**
     * Callback types for TabBar events
     */
    using TabClickCallback = std::function<void(int tabIndex, const Tab& tab)>;
    using TabHoverCallback = std::function<void(int tabIndex, const Tab& tab)>;
    using TabRightClickCallback = std::function<void(int tabIndex, const Tab& tab)>;
    using TabDoubleClickCallback = std::function<void(int tabIndex, const Tab& tab)>;  // Double-click
    using AddTabCallback = std::function<void()>;  // Called when add button clicked

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
        // TAB BOUNDS QUERIES
        // ========================================================================

        /**
         * Get the bounds of a specific tab (after render())
         * Returns nullptr if index is invalid or render() hasn't been called yet
         */
        const TabBounds* getTabBounds(int index) const;

        /**
         * Get all tab bounds (after render())
         */
        const std::vector<TabBounds>& getAllTabBounds() const { return tabBounds_; }

        /**
         * Find which tab contains a screen position
         * Returns tab index, or -1 if no tab at that position
         */
        int getTabAtPosition(float x, float y) const;

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

        /**
         * Set callback for when a tab is double-clicked
         */
        void setOnTabDoubleClick(TabDoubleClickCallback callback) { onTabDoubleClick_ = callback; }

        /**
         * Set callback for when the add button is clicked
         */
        void setOnAddTab(AddTabCallback callback) { onAddTab_ = callback; }

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

        // Tab bounds storage
        std::vector<TabBounds> tabBounds_;  // Stored after each render

        // Callbacks
        TabClickCallback onTabClick_;
        TabHoverCallback onTabHover_;
        TabRightClickCallback onTabRightClick_;
        TabDoubleClickCallback onTabDoubleClick_;
        AddTabCallback onAddTab_;

        // Internal state
        int hoveredTabIndex_;  // Currently hovered tab (-1 = none)

        // Helper methods
        void renderTab(int index, const Tab& tab, bool isActive);
        void renderAddButton();
        void renderBadge(int badgeCount);
        ImVec2 calculateTabSize(const Tab& tab) const;
        void applyTabStyle(bool isActive, bool isHovered, bool isDisabled);
        void restoreStyle();
    };

} // namespace elda::ui

#endif