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
        void* user_data = nullptr;   // Optional user data pointer
        bool enabled = true;         // Whether tab is clickable
        TabBounds* bounds = nullptr; // Bounds of the tab (updated after render)

        Tab() = default;

        explicit Tab(const std::string& label_)
            : label(label_), id(label_) {}

        Tab(const std::string& label_, int badge_)
            : label(label_), id(label_), badge(badge_) {}

        Tab(const std::string& label_, const std::string& id_)
            : label(label_), id(id_) {}

        Tab(const std::string& label_, int badge_, void* user_data_)
            : label(label_), id(label_), badge(badge_), user_data(user_data_) {}
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
    ImVec4 active_color        = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // Active tab (darker)
    ImVec4 inactive_color      = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);  // Inactive tab
    ImVec4 hover_color         = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);  // Hover state
    ImVec4 disabled_color      = ImVec4(0.10f, 0.10f, 0.10f, 0.50f);  // Disabled tab
    ImVec4 badge_color         = ImVec4(0.89f, 0.33f, 0.30f, 1.00f);  // Badge background
    ImVec4 badge_text_color    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // Badge text
    ImVec4 border_color        = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);  // Tab border
    ImVec4 background_color    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // Tab bar background
    ImVec4 separator_color     = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);  // Bottom separator

    // Sizing
    float height               = 44.0f;     // Tab bar height (includes padding)
    float button_padding_x     = 20.0f;     // Horizontal padding inside tab
    float button_padding_y     = 10.0f;     // Vertical padding inside tab
    float spacing              = 1.2f;      // Space between tabs (browser-style tight spacing)
    float rounding             = 6.0f;      // Corner rounding (top corners only)
    float badge_offset_x       = 8.0f;      // Badge offset from label
    float separator_height     = 1.0f;      // Height of separator line below tabs
    float border_thickness     = 1.0f;      // Border thickness around tabs
    float tab_bar_padding      = 4.0f;      // Padding around tab bar area
    float add_button_icon_size = 0.3f;      // Size of + icon relative to button (0.1-0.5)

    // Layout
    bool show_separator        = true;      // Show separator line below tabs
    bool auto_size             = false;     // Auto-size tabs to fill width
    bool show_badges           = true;      // Show badge counts
    bool browser_style         = true;      // Use browser-style (rounded top, square bottom)
    bool show_borders          = true;      // Show borders around tabs
    bool show_background       = true;      // Show tab bar background
    bool show_add_button       = false;     // Show add (+) button after tabs

    TabBarStyle() = default;
};

    /**
     * Callback types for TabBar events
     */
    using TabClickCallback = std::function<void(int tab_index, const Tab& tab)>;
    using TabHoverCallback = std::function<void(int tab_index, const Tab& tab)>;
    using TabRightClickCallback = std::function<void(int tab_index, const Tab& tab)>;
    using TabDoubleClickCallback = std::function<void(int tab_index, const Tab& tab)>;
    using AddTabCallback = std::function<void()>;  // Called when add button clicked

    /**
     * Reusable TabBar Component
     */
    class TabBar {
    public:
        TabBar();
        explicit TabBar(const TabBarStyle& style);
        ~TabBar() = default;

        // ========================================================================
        // CONFIGURATION
        // ========================================================================

        void set_tabs(const std::vector<Tab>& tabs);
        void set_active_tab(int index);
        int  get_active_tab() const { return active_tab_index_; }

        void set_style(const TabBarStyle& style);
        TabBarStyle& get_style() { return style_; }

        void add_tab(const Tab& tab);
        void remove_tab(int index);
        void clear();
        void set_badge(int tab_index, int badge);
        void set_tab_enabled(int tab_index, bool enabled);

        // ========================================================================
        // TAB BOUNDS QUERIES
        // ========================================================================

        const TabBounds* get_tab_bounds(int index) const;
        const std::vector<TabBounds>& get_all_tab_bounds() const { return tab_bounds_; }
        int get_tab_at_position(float x, float y) const;

        // ========================================================================
        // CALLBACKS
        // ========================================================================

        void set_on_tab_click(TabClickCallback callback) { on_tab_click_ = callback; }
        void set_on_tab_hover(TabHoverCallback callback) { on_tab_hover_ = callback; }
        void set_on_tab_right_click(TabRightClickCallback callback) { on_tab_right_click_ = callback; }
        void set_on_tab_double_click(TabDoubleClickCallback callback) { on_tab_double_click_ = callback; }
        void set_on_add_tab(AddTabCallback callback) { on_add_tab_ = callback; }

        // ========================================================================
        // RENDERING
        // ========================================================================

        bool render();

        size_t get_tab_count() const { return tabs_.size(); }
        bool   is_empty() const { return tabs_.empty(); }
        const Tab* get_tab(int index) const;

    private:
        std::vector<Tab> tabs_;
        int active_tab_index_;
        TabBarStyle style_;

        std::vector<TabBounds> tab_bounds_;  // Stored after each render

        TabClickCallback       on_tab_click_;
        TabHoverCallback       on_tab_hover_;
        TabRightClickCallback  on_tab_right_click_;
        TabDoubleClickCallback on_tab_double_click_;
        AddTabCallback         on_add_tab_;

        int hovered_tab_index_;  // Currently hovered tab (-1 = none)

        void render_tab(int index, const Tab& tab, bool is_active);
        void render_add_button();
        void render_badge(int badge_count);
        ImVec2 calculate_tab_size(const Tab& tab) const;
        void apply_tab_style(bool is_active, bool is_hovered, bool is_disabled);
        void restore_style();
    };

}