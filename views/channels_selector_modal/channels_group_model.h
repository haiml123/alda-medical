#pragma once

#include "../../models/channel.h"
#include "../../models/channels_group.h"
#include "../../models/mvp_base_model.h"
#include "../../services/channel_management_service.h"
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace elda::channels_group {

    /**
     * Model: Contains all data and business logic for channel selection
     * No UI dependencies - pure data and logic
     *
     * ✅ UPDATED: Now extends MVPBaseModel for state management integration
     * ✅ UPDATED: Tracks groupId_ to distinguish edit vs create
     * ✅ UPDATED: Notifies parent when groups change via callback
     */
    class ChannelsGroupModel : public elda::models::MVPBaseModel {
    public:
        // Callback to notify parent when available groups need refresh
        using OnGroupsChangedCallback = std::function<void()>;

        explicit ChannelsGroupModel(elda::AppStateManager& stateManager);

        // ========================================================================
        // DATA ACCESS
        // ========================================================================

        const std::vector<models::Channel>& GetChannels() const { return channels_; }
        const std::string& GetGroupName() const { return groupName_; }
        const std::string& GetGroupId() const { return groupId_; }
        int GetSelectedCount() const;
        int GetTotalCount() const { return static_cast<int>(channels_.size()); }

        // ========================================================================
        // DATA MODIFICATION
        // ========================================================================

        void SetGroupName(const std::string& name) { groupName_ = name; }
        void SetChannels(const std::vector<models::Channel>& channels);
        void SelectChannel(size_t index, bool selected);
        void SelectAllChannels(bool selected);

        // ========================================================================
        // CALLBACK REGISTRATION
        // ========================================================================

        /**
         * Set callback to be invoked when groups list changes (create/update/delete)
         * @param callback Function to call when refresh is needed
         */
        void SetOnGroupsChangedCallback(OnGroupsChangedCallback callback) {
            onGroupsChangedCallback_ = callback;
        }

        // ========================================================================
        // VALIDATION
        // ========================================================================

        bool CanConfirm(std::string& errorMessage) const;

        // ========================================================================
        // BUSINESS LOGIC - Channel Group Management
        // ========================================================================

        /**
         * Load a channel group by ID from the service
         * @param id ID of the group to load
         * @return true if loaded successfully
         */
        bool LoadChannelGroupById(const std::string& id);

        /**
         * Load the active/last used channel group
         * @return true if loaded successfully
         */
        bool LoadActiveChannelGroup();

        /**
         * Save the current selection as a new or updated channel group
         * ✅ Automatically triggers groups changed callback on success
         * @return true if saved successfully
         */
        bool SaveChannelGroup();

        /**
         * Delete the current channel group from the service
         * ✅ Automatically triggers groups changed callback on success
         * @return true if deleted successfully
         */
        bool DeleteChannelGroup();

        /**
         * Get all available channels
         * @return Vector of channels
         */
        std::vector<models::Channel> GetAllChannels() const;

        /**
         * Get all available channel group names
         * @return Vector of group names
         */
        std::vector<std::string> GetAvailableGroupNames() const;

        /**
         * Clear current data (reset state)
         */
        void Clear();

        /**
         * Check if this is a new group (doesn't exist in service yet)
         * @return true if group doesn't exist or name is empty
         */
        bool IsNewGroup() const;

    private:
        std::vector<models::Channel> channels_;
        std::string groupName_;
        std::string groupId_;
        services::ChannelManagementService& channelService_;
        OnGroupsChangedCallback onGroupsChangedCallback_;

        /**
         * Notify parent that groups have changed and need refresh
         */
        void NotifyGroupsChanged();
    };

} // namespace elda::channels_group