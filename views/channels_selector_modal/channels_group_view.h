#pragma once

#include "imgui.h"
#include "../../models/channel.h"
#include <string>
#include <vector>
#include <functional>

namespace elda::views::channels_selector {

    /**
     * View interface: Defines callbacks that the Presenter will implement
     * The View calls these when user interacts with UI
     */
    struct IChannelsGroupViewCallbacks {
        std::function<void(const std::string&)> on_group_name_changed;
        std::function<void(size_t, bool)> on_channel_selection_changed;
        std::function<void(bool)> on_select_all_channels;
        std::function<void()> on_confirm;
        std::function<void()> on_cancel;
        std::function<void()> on_delete;
    };

    /**
     * View: Pure UI rendering with no business logic
     * Receives data to display and callbacks to invoke on user actions
     */
    class ChannelsGroupView {
    public:
        ChannelsGroupView();

        // ========================================================================
        // VIEW STATE
        // ========================================================================

        void set_visible(bool visible) { is_visible_ = visible; }
        bool is_visible() const { return is_visible_; }

        void set_position(ImVec2 pos) { position_ = pos; }
        void set_size(ImVec2 size) { size_ = size; }

        // ========================================================================
        // CALLBACKS
        // ========================================================================

        void set_callbacks(const IChannelsGroupViewCallbacks& callbacks) {
            callbacks_ = callbacks;
        }

        // ========================================================================
        // RENDERING
        // ========================================================================

        /**
         * Render the modal window
         * @param groupName Current group name
         * @param channels List of channels to display
         * @param selectedCount Number of selected channels
         * @param totalCount Total number of channels
         * @param canConfirm Whether confirm button should be enabled
         * @param isNewGroup Whether this is a new group (affects Delete/Cancel button)
         */
        void render(
            const std::string& group_name,
            const std::vector<models::Channel>& channels,
            int selected_count,
            int total_count,
            bool can_confirm,
            bool is_new_group
        );

    private:
        void render_header(const std::string& group_name);
        void render_channels_list(const std::vector<models::Channel>& channels, int selected_count, int total_count);
        void render_footer(bool can_confirm, bool is_new_group);

        // Custom checkbox rendering
        bool render_custom_checkbox(const char* label, bool value);

        bool is_visible_;
        ImVec2 position_;
        ImVec2 size_;
        IChannelsGroupViewCallbacks callbacks_;

        // Internal state for input (view-only state, not business data)
        char name_input_buffer_[256];
    };

} // namespace elda::channels_group