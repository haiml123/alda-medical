#pragma once

#include "imgui.h"
#include "../../models/channel.h"
#include <string>
#include <vector>
#include <functional>

namespace elda::channels_group {

    /**
     * View interface: Defines callbacks that the Presenter will implement
     * The View calls these when user interacts with UI
     */
    struct IChannelsGroupViewCallbacks {
        std::function<void(const std::string&)> onGroupNameChanged;
        std::function<void(size_t, bool)> onChannelSelectionChanged;
        std::function<void(bool)> onSelectAllChannels;
        std::function<void()> onConfirm;
        std::function<void()> onCancel;
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

        void SetVisible(bool visible) { isVisible_ = visible; }
        bool IsVisible() const { return isVisible_; }

        void SetPosition(ImVec2 pos) { position_ = pos; }
        void SetSize(ImVec2 size) { size_ = size; }

        // ========================================================================
        // CALLBACKS
        // ========================================================================

        void SetCallbacks(const IChannelsGroupViewCallbacks& callbacks) {
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
         */
        void Render(
            const std::string& groupName,
            const std::vector<models::Channel>& channels,
            int selectedCount,
            int totalCount,
            bool canConfirm
        );

    private:
        void RenderHeader(const std::string& groupName);
        void RenderChannelsList(const std::vector<models::Channel>& channels, int selectedCount, int totalCount);
        void RenderFooter(bool canConfirm);

        // Custom checkbox rendering
        bool RenderCustomCheckbox(const char* label, bool value);

        bool isVisible_;
        ImVec2 position_;
        ImVec2 size_;
        IChannelsGroupViewCallbacks callbacks_;

        // Internal state for input (view-only state, not business data)
        char nameInputBuffer_[256];
    };

} // namespace elda::channels_group