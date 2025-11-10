#pragma once

#include "channels_group_model.h"
#include "channels_group_view.h"
#include "../../core/app_state_manager.h"
#include <memory>
#include <functional>

namespace elda::channels_group {

    /**
     * Presenter: Mediates between Model and View
     * - Responds to View events (user actions)
     * - Updates Model based on user input
     * - Updates View based on Model state
     * - Contains presentation logic
     *
     * ✅ UPDATED: Now requires AppStateManager for MVPBaseModel integration
     * ✅ UPDATED: Supports refresh callback for parent to reload groups
     */
    class ChannelsGroupPresenter {
    public:
        using OnConfirmCallback = std::function<void(const models::ChannelsGroup&)>;
        using OnDeleteCallback = std::function<void(const std::string&)>;
        using OnGroupsChangedCallback = std::function<void()>;

        explicit ChannelsGroupPresenter(elda::AppStateManager& stateManager);
        ~ChannelsGroupPresenter() = default;

        // ========================================================================
        // PUBLIC API - Called by Application
        // ========================================================================

        /**
         * Open the modal with channels from a specific group (by ID)
         * @param groupId ID of the group to load (empty = new group)
         * @param callback Callback when user confirms
         * @param deleteCallback Callback when user deletes (optional)
         */
        void Open(
            const std::string& groupId = "",
            OnConfirmCallback callback = nullptr,
            OnDeleteCallback deleteCallback = nullptr
        );

        /**
         * Set callback to be invoked whenever groups list changes
         * This allows parent to refresh its view of available groups
         * @param callback Function to call when groups need refresh
         */
        void SetOnGroupsChangedCallback(OnGroupsChangedCallback callback);

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
        void OnDelete();

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
        OnDeleteCallback onDeleteCallback_;
    };

} // namespace elda::channels_group