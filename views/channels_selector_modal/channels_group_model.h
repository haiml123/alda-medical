#pragma once

#include "../../models/channel.h"
#include "../../models/channels_group.h"
#include "../../services/channel_management_service.h"
#include <vector>
#include <string>
#include <optional>

namespace elda::channels_group {

    /**
     * Model: Contains all data and business logic for channel selection
     * No UI dependencies - pure data and logic
     */
    class ChannelsGroupModel {
    public:
        ChannelsGroupModel();

        // ========================================================================
        // DATA ACCESS
        // ========================================================================

        const std::vector<models::Channel>& GetChannels() const { return channels_; }
        const std::string& GetGroupName() const { return groupName_; }
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
        // VALIDATION
        // ========================================================================

        bool CanConfirm(std::string& errorMessage) const;

        // ========================================================================
        // BUSINESS LOGIC - Channel Group Management
        // ========================================================================

        /**
         * Load a channel group by name from the service
         * @param groupName Name of the group to load
         * @return true if loaded successfully
         */
        bool LoadChannelGroup(const std::string& groupName);

        /**
         * Load the active/last used channel group
         * @return true if loaded successfully
         */
        bool LoadActiveChannelGroup();

        /**
         * Save the current selection as a new or updated channel group
         * @return true if saved successfully
         */
        bool SaveChannelGroup();

        /**
         * Get all available channel group names
         * @return Vector of group names
         */
        std::vector<std::string> GetAvailableGroupNames() const;

        /**
         * Clear current data (reset state)
         */
        void Clear();

    private:
        std::vector<models::Channel> channels_;
        std::string groupName_;
        services::ChannelManagementService& channelService_;
    };

} // namespace elda::channels_group