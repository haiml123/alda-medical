#pragma once

#include "channels_group_model.h"
#include "channels_group_view.h"
#include <memory>
#include <functional>

namespace elda::channels_group {

    /**
     * Presenter: Mediates between Model and View
     * - Responds to View events (user actions)
     * - Updates Model based on user input
     * - Updates View based on Model state
     * - Contains presentation logic
     */
    class ChannelsGroupPresenter {
    public:
        using OnConfirmCallback = std::function<void(const models::ChannelsGroup&)>;

        ChannelsGroupPresenter();
        ~ChannelsGroupPresenter() = default;

        // ========================================================================
        // PUBLIC API - Called by Application
        // ========================================================================

        /**
         * Open the modal with channels from a specific group
         * @param groupName Name of the group to load
         * @param callback Callback when user confirms
         */
        void Open(const std::string& groupName, OnConfirmCallback callback = nullptr);

        /**
         * Open the modal with the active/last used group
         * @param callback Callback when user confirms
         */
        void OpenWithActiveGroup(OnConfirmCallback callback = nullptr);

        /**
         * Open the modal with custom channels (not from service)
         * @param channels Channels to display
         * @param callback Callback when user confirms
         */
        void OpenWithChannels(const std::vector<models::Channel>& channels,
                            OnConfirmCallback callback = nullptr);

        /**
         * Close the modal
         */
        void Close();

        /**
         * Render the modal (call every frame)
         * @param buttonPos Position to place the modal (typically below a button)
         */
        void Render(ImVec2 buttonPos);

        /**
         * Check if modal is currently open
         */
        bool IsOpen() const;

    private:
        // ========================================================================
        // VIEW EVENT HANDLERS - Respond to user actions
        // ========================================================================

        void OnGroupNameChanged(const std::string& newName);
        void OnChannelSelectionChanged(size_t index, bool selected);
        void OnSelectAllChannels(bool selected);
        void OnConfirm();
        void OnCancel();

        // ========================================================================
        // INTERNAL HELPERS
        // ========================================================================

        void UpdateView();
        void SetupViewCallbacks();

        // ========================================================================
        // MEMBERS
        // ========================================================================

        std::unique_ptr<ChannelsGroupModel> model_;
        std::unique_ptr<ChannelsGroupView> view_;
        OnConfirmCallback onConfirmCallback_;
    };

} // namespace elda::channels_group