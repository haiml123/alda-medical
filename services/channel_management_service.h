#pragma once

#include "../models/channel.h"
#include "../models/channels_group.h"
#include "secure_storage_service.h"
#include <vector>
#include <string>
#include <optional>
#include <memory>

namespace elda::services {

    /**
     * Unified service for managing both individual channels and channel groups (montages).
     * Provides CRUD operations with persistent storage and validation.
     *
     * Similar to NeoRec's profile management where channels can be grouped into montages/profiles.
     */
    class ChannelManagementService {
    public:
        // ============================================================================
        // CHANNEL OPERATIONS
        // ============================================================================

        /**
         * Create a new channel
         * @param channel Channel to create
         * @return true if created successfully
         */
        bool CreateChannel(const models::Channel& channel);

        /**
         * Get a channel by ID
         * @param id Channel unique identifier
         * @return Optional channel if found
         */
        std::optional<models::Channel> GetChannel(const std::string& id) const;

        /**
         * Get all channels
         * @return Vector of all channels
         */
        std::vector<models::Channel> GetAllChannels() const;

        /**
         * Update an existing channel
         * @param channel Channel with updated data (must have valid id)
         * @return true if updated successfully
         */
        bool UpdateChannel(const models::Channel& channel);

        /**
         * Delete a channel by ID
         * @param id Channel identifier
         * @return true if deleted successfully
         */
        bool DeleteChannel(const std::string& id);

        /**
         * Check if a channel exists
         * @param id Channel identifier
         * @return true if channel exists
         */
        bool ChannelExists(const std::string& id) const;

        // ============================================================================
        // CHANNEL GROUP (MONTAGE) OPERATIONS
        // ============================================================================

        /**
         * Create a new channel group/montage
         * @param group Channel group to create
         * @return true if created successfully
         */
        bool CreateChannelGroup(const models::ChannelsGroup& group);

        /**
         * Get a channel group by name
         * @param name Group name
         * @return Optional channel group if found
         */
        std::optional<models::ChannelsGroup> GetChannelGroup(const std::string& name) const;

        /**
         * Get all channel groups
         * @return Vector of all channel groups
         */
        std::vector<models::ChannelsGroup> GetAllChannelGroups() const;

        /**
         * Update an existing channel group
         * @param group Channel group with updated data (must have valid name)
         * @return true if updated successfully
         */
        bool UpdateChannelGroup(const models::ChannelsGroup& group);

        /**
         * Delete a channel group by name
         * @param name Group name
         * @return true if deleted successfully
         */
        bool DeleteChannelGroup(const std::string& name);

        /**
         * Check if a channel group exists
         * @param name Group name
         * @return true if group exists
         */
        bool ChannelGroupExists(const std::string& name) const;

        /**
         * Rename a channel group
         * @param oldName Current group name
         * @param newName New group name
         * @return true if renamed successfully
         */
        bool RenameChannelGroup(const std::string& oldName, const std::string& newName);

        // ============================================================================
        // LAST USED / ACTIVE MANAGEMENT
        // ============================================================================

        /**
         * Save the currently active channel group as "last used"
         * @param group The active channel group
         * @return true if saved successfully
         */
        bool SaveActiveChannelGroup(const models::ChannelsGroup& group);

        /**
         * Load the last used/active channel group
         * @return Optional channel group if available
         */
        std::optional<models::ChannelsGroup> LoadActiveChannelGroup() const;

        // ============================================================================
        // VALIDATION & UTILITIES
        // ============================================================================

        /**
         * Validate a channel group (e.g., check for duplicate channel IDs)
         * @param group Group to validate
         * @param errorMessage Output parameter for error details
         * @return true if valid
         */
        bool ValidateChannelGroup(const models::ChannelsGroup& group, std::string& errorMessage) const;

        /**
         * Get default/system channel groups (like 10-20, 10-10 systems)
         * @return Vector of default groups
         */
        std::vector<models::ChannelsGroup> GetDefaultChannelGroups() const;

        // ============================================================================
        // SINGLETON ACCESS
        // ============================================================================

        static ChannelManagementService& GetInstance();

        // Prevent copying
        ChannelManagementService(const ChannelManagementService&) = delete;
        ChannelManagementService& operator=(const ChannelManagementService&) = delete;

    private:
        ChannelManagementService();
        ~ChannelManagementService() = default;

        // Storage helpers
        void LoadFromStorage();
        void SaveToStorage();

        // Internal data
        std::vector<models::Channel> channels_;
        std::vector<models::ChannelsGroup> channelGroups_;

        // Storage services
        std::unique_ptr<elda::services::SecureConfigManager<std::vector<models::Channel>>> channelStorage_;
        std::unique_ptr<elda::services::SecureConfigManager<std::vector<models::ChannelsGroup>>> groupStorage_;
        std::unique_ptr<elda::services::SecureConfigManager<models::ChannelsGroup>> activeGroupStorage_;

        // Helper methods
        std::vector<models::Channel>::iterator FindChannelById(const std::string& id);
        std::vector<models::Channel>::const_iterator FindChannelById(const std::string& id) const;
        std::vector<models::ChannelsGroup>::iterator FindGroupByName(const std::string& name);
        std::vector<models::ChannelsGroup>::const_iterator FindGroupByName(const std::string& name) const;

        bool InitializeDefaultChannels();
    };

} // namespace elda::services