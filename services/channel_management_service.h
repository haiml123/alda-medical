#pragma once

#include "../models/channel.h"
#include "../models/channels_group.h"
#include "secure_storage_service.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace elda::services
{

/**
 * Unified service for managing both individual channels and channel groups (montages).
 * Provides CRUD operations with persistent storage and validation.
 *
 * Similar to NeoRec's profile management where channels can be grouped into montages/profiles.
 *
 * IMPORTANT: All group operations use ID-based lookups, not name-based.
 * This allows renaming groups without creating duplicates.
 */
class ChannelManagementService
{
  public:
    // ============================================================================
    // CHANNEL OPERATIONS
    // ============================================================================

    /**
     * Create a new channel
     * @param channel Channel to create
     * @return true if created successfully
     */
    bool create_channel(const models::Channel& channel);

    /**
     * Get a channel by ID
     * @param id Channel unique identifier
     * @return Optional channel if found
     */
    std::optional<models::Channel> get_channel(const std::string& id) const;

    /**
     * Get all channels
     * @return Vector of all channels
     */
    const std::vector<models::Channel>& get_all_channels() const;

    /**
     * Update an existing channel
     * @param channel Channel with updated data (must have valid id)
     * @return true if updated successfully
     */
    bool update_channel(const models::Channel& channel);

    /**
     * Delete a channel by ID
     * @param id Channel identifier
     * @return true if deleted successfully
     */
    bool delete_channel(const std::string& id);

    /**
     * Check if a channel exists
     * @param id Channel identifier
     * @return true if channel exists
     */
    bool channel_exists(const std::string& id) const;

    // ============================================================================
    // CHANNEL GROUP (MONTAGE) OPERATIONS - ID-BASED
    // ============================================================================

    /**
     * Create a new channel group/montage
     * @param group Channel group to create (must have unique id)
     * @return true if created successfully
     */
    bool create_channel_group(const models::ChannelsGroup& group);

    /**
     * Get a channel group by ID
     * @param id Group unique identifier
     * @return Optional channel group if found
     */
    std::optional<models::ChannelsGroup> get_channel_group(const std::string& id) const;

    /**
     * Get a channel group by name (for backward compatibility)
     * @param name Group name
     * @return Optional channel group if found
     */
    std::optional<models::ChannelsGroup> get_channel_group_by_name(const std::string& name) const;

    /**
     * Get all channel groups
     * @return Vector of all channel groups
     */
    std::vector<models::ChannelsGroup> get_all_channel_groups() const;

    /**
     * Update an existing channel group
     * @param group Channel group with updated data (must have valid id)
     * @return true if updated successfully
     */
    bool update_channel_group(const models::ChannelsGroup& group);

    /**
     * Delete a channel group by ID
     * @param id Group identifier
     * @return true if deleted successfully
     */
    bool delete_channel_group(const std::string& id);

    /**
     * Delete all non-default channel groups
     * @return Number of groups deleted
     */
    int delete_all_channel_groups();

    /**
     * Check if a channel group exists by ID
     * @param id Group identifier
     * @return true if group exists
     */
    bool channel_group_exists(const std::string& id) const;

    /**
     * Check if a channel group exists by name
     * @param name Group name
     * @return true if group exists
     */
    bool channel_group_exists_by_name(const std::string& name) const;

    // ============================================================================
    // LAST USED / ACTIVE MANAGEMENT
    // ============================================================================

    /**
     * Save the currently active channel group as "last used"
     * @param group The active channel group
     * @return true if saved successfully
     */
    bool save_active_channel_group(const models::ChannelsGroup& group);

    /**
     * Load the last used/active channel group
     * @return Optional channel group if available
     */
    std::optional<models::ChannelsGroup> load_active_channel_group() const;

    // ============================================================================
    // VALIDATION & UTILITIES
    // ============================================================================

    /**
     * Validate a channel group (e.g., check for duplicate channel IDs)
     * @param group Group to validate
     * @param error_message Output parameter for error details
     * @return true if valid
     */
    bool validate_channel_group(const models::ChannelsGroup& group, std::string& error_message) const;

    /**
     * Get default/system channel groups (like 10-20, 10-10 systems)
     * @return Vector of default groups
     */
    std::vector<models::ChannelsGroup> get_default_channel_groups() const;

    // ============================================================================
    // SINGLETON ACCESS
    // ============================================================================

    static ChannelManagementService& get_instance();

    // Prevent copying
    ChannelManagementService(const ChannelManagementService&) = delete;
    ChannelManagementService& operator=(const ChannelManagementService&) = delete;

  private:
    ChannelManagementService();
    ~ChannelManagementService() = default;

    // Storage helpers
    void load_from_storage();
    void save_to_storage();

    // Internal data
    std::vector<models::Channel> channels_;
    std::vector<models::ChannelsGroup> channel_groups_;

    // Storage services
    std::unique_ptr<elda::services::SecureConfigManager<std::vector<models::Channel>>> channel_storage_;
    std::unique_ptr<elda::services::SecureConfigManager<std::vector<models::ChannelsGroup>>> group_storage_;
    std::unique_ptr<elda::services::SecureConfigManager<models::ChannelsGroup>> active_group_storage_;

    // Helper methods
    std::vector<models::Channel>::iterator find_channel_by_id(const std::string& id);
    std::vector<models::Channel>::const_iterator find_channel_by_id(const std::string& id) const;

    // ID-based lookups (primary)
    std::vector<models::ChannelsGroup>::iterator find_group_by_id(const std::string& id);
    std::vector<models::ChannelsGroup>::const_iterator find_group_by_id(const std::string& id) const;

    // Name-based lookups (for backward compatibility)
    std::vector<models::ChannelsGroup>::iterator find_group_by_name(const std::string& name);
    std::vector<models::ChannelsGroup>::const_iterator find_group_by_name(const std::string& name) const;

    bool initialize_default_channels();
};

}  // namespace elda::services
