#pragma once

#include "../../models/channel.h"
#include "../../models/channels_group.h"
#include "../../models/mvp_base_model.h"
#include "../../services/channel_management_service.h"

#include <functional>
#include <string>
#include <vector>

namespace elda::views::channels_selector
{

/**
 * Model: Contains all data and business logic for channel selection
 * No UI dependencies - pure data and logic
 *
 */
class ChannelsGroupModel : public elda::models::MVPBaseModel
{
  public:
    // Callback to notify parent when available groups need refresh
    using OnGroupsChangedCallback = std::function<void()>;

    explicit ChannelsGroupModel(elda::AppStateManager& state_manager);

    // ========================================================================
    // DATA ACCESS
    // ========================================================================

    const std::vector<models::Channel>& get_channels() const
    {
        return channels_;
    }
    const std::string& get_group_name() const
    {
        return group_name_;
    }
    const std::string& get_group_id() const
    {
        return group_id_;
    }
    int get_selected_count() const;
    int get_total_count() const
    {
        return static_cast<int>(channels_.size());
    }

    // ========================================================================
    // DATA MODIFICATION
    // ========================================================================

    void set_group_name(const std::string& name)
    {
        group_name_ = name;
    }
    void set_channels(const std::vector<models::Channel>& channels);
    void select_channel(size_t index, bool selected);
    void select_all_channels(bool selected);

    // ========================================================================
    // CALLBACK REGISTRATION
    // ========================================================================

    /**
     * Set callback to be invoked when groups list changes (create/update/delete)
     * @param callback Function to call when refresh is needed
     */
    void set_on_groups_changed_callback(OnGroupsChangedCallback callback)
    {
        on_groups_changed_callback_ = callback;
    }

    // ========================================================================
    // VALIDATION
    // ========================================================================

    bool can_confirm(std::string& error_message) const;

    // ========================================================================
    // BUSINESS LOGIC - Channel Group Management
    // ========================================================================

    /**
     * Load a channel group by ID from the service
     * @param id ID of the group to load
     * @return true if loaded successfully
     */
    bool load_channel_group_by_id(const std::string& id);

    /**
     * Load the active/last used channel group
     * @return true if loaded successfully
     */
    bool load_active_channel_group();

    /**
     * Save the current selection as a new or updated channel group
     * @return true if saved successfully
     */
    bool save_channel_group();

    /**
     * Delete the current channel group from the service
     * @return true if deleted successfully
     */
    bool delete_channel_group();

    /**
     * Get all available channels
     * @return Vector of channels
     */
    std::vector<models::Channel> get_all_channels() const;

    /**
     * Get all available channel group names
     * @return Vector of group names
     */
    std::vector<std::string> get_available_group_names() const;

    /**
     * Clear current data (reset state)
     */
    void clear();

    /**
     * Check if this is a new group (doesn't exist in service yet)
     * @return true if group doesn't exist or name is empty
     */
    bool is_new_group() const;

  private:
    std::vector<models::Channel> channels_;
    std::string group_name_;
    std::string group_id_;
    services::ChannelManagementService& channel_service_;
    OnGroupsChangedCallback on_groups_changed_callback_;

    /**
     * Notify parent that groups have changed and need refresh
     */
    void notify_groups_changed();
};

}  // namespace elda::views::channels_selector