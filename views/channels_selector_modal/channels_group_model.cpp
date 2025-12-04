#include "channels_group_model.h"

#include <algorithm>

namespace elda::views::channels_selector
{

ChannelsGroupModel::ChannelsGroupModel(elda::AppStateManager& state_manager)
    : MVPBaseModel(state_manager),
      channel_service_(services::ChannelManagementService::get_instance()),
      on_groups_changed_callback_(nullptr)
{
}

// ========================================================================
// DATA ACCESS
// ========================================================================

int ChannelsGroupModel::get_selected_count() const
{
    return std::count_if(channels_.begin(),
                         channels_.end(),
                         [](const models::Channel& ch)
                         {
                             return ch.selected;
                         });
}

// ========================================================================
// DATA MODIFICATION
// ========================================================================

void ChannelsGroupModel::set_channels(const std::vector<models::Channel>& channels)
{
    channels_ = channels;
}

void ChannelsGroupModel::select_channel(size_t index, bool selected)
{
    if (index < channels_.size())
    {
        channels_[index].selected = selected;
    }
}

void ChannelsGroupModel::select_all_channels(bool selected)
{
    for (auto& ch : channels_)
    {
        ch.selected = selected;
    }
}

// ========================================================================
// VALIDATION
// ========================================================================

bool ChannelsGroupModel::can_confirm(std::string& error_message) const
{
    if (group_name_.empty())
    {
        error_message = "Group name cannot be empty";
        return false;
    }

    if (get_selected_count() == 0)
    {
        error_message = "At least one channel must be selected";
        return false;
    }

    return true;
}

// ========================================================================
// BUSINESS LOGIC
// ========================================================================

bool ChannelsGroupModel::load_channel_group_by_id(const std::string& id)
{
    auto group = channel_service_.get_channel_group(id);
    if (group.has_value())
    {
        group_id_ = group->id;  // ✅ CRITICAL: Store the ID from BaseModel
        group_name_ = group->name;

        channels_ = channel_service_.get_all_channels();

        // Mark channels that are in the group as selected
        for (auto& channel : channels_)
        {
            channel.selected = std::find(group->channel_ids.begin(), group->channel_ids.end(), channel.get_id()) !=
                               group->channel_ids.end();
        }

        return true;
    }
    return false;
}

bool ChannelsGroupModel::load_active_channel_group()
{
    auto group = channel_service_.load_active_channel_group();
    if (group.has_value())
    {
        group_id_ = group->id;
        group_name_ = group->name;

        channels_ = channel_service_.get_all_channels();

        // Mark channels that are in the group as selected
        for (auto& channel : channels_)
        {
            channel.selected = std::find(group->channel_ids.begin(), group->channel_ids.end(), channel.get_id()) !=
                               group->channel_ids.end();
        }

        return true;
    }
    return false;
}

bool ChannelsGroupModel::save_channel_group()
{
    // TODO: This should go through AppStateManager for:
    // - State validation
    // - Audit logging
    // - Observer notifications
    // - Medical device compliance
    // See APPSTATE_INTEGRATION.cpp for implementation options

    std::vector<std::string> selected_channel_ids;
    for (const auto& channel : channels_)
    {
        if (channel.selected)
        {
            selected_channel_ids.push_back(channel.id);
        }
    }

    bool success = false;

    if (group_id_.empty())
    {
        // CREATE: New group
        models::ChannelsGroup group(group_name_);
        group.channel_ids = selected_channel_ids;
        group.on_create();

        if (channel_service_.create_channel_group(group))
        {
            // Remember the ID for future operations
            group_id_ = group.id;
            success = true;

            std::printf("[ChannelsGroupModel] ✓ Created group: %s (ID: %s)\n", group_name_.c_str(), group_id_.c_str());
        }
        else
        {
            std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to create group: %s\n", group_name_.c_str());
        }
    }
    else
    {
        // UPDATE: Existing group - preserve ID, allow name change
        models::ChannelsGroup group(group_id_, group_name_);
        group.channel_ids = selected_channel_ids;
        group.on_update();

        if (channel_service_.update_channel_group(group))
        {
            success = true;

            std::printf("[ChannelsGroupModel] ✓ Updated group: %s (ID: %s)\n", group_name_.c_str(), group_id_.c_str());
        }
        else
        {
            std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to update group: %s\n", group_name_.c_str());
        }
    }

    if (success)
    {
        // Save as active group
        models::ChannelsGroup active_group(group_id_, group_name_);
        active_group.channel_ids = selected_channel_ids;
        channel_service_.save_active_channel_group(active_group);

        notify_groups_changed();
    }

    return success;
}

bool ChannelsGroupModel::delete_channel_group()
{
    if (group_id_.empty())
    {
        return false;  // Can't delete a group that doesn't exist yet
    }

    // TODO: This should go through AppStateManager for:
    // - State validation
    // - Audit logging
    // - Observer notifications
    // - Medical device compliance
    // See APPSTATE_INTEGRATION.cpp for implementation options

    if (channel_service_.delete_channel_group(group_id_))
    {
        std::printf("[ChannelsGroupModel] ✓ Deleted group: %s (ID: %s)\n", group_name_.c_str(), group_id_.c_str());

        notify_groups_changed();

        return true;
    }

    std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to delete group: %s\n", group_name_.c_str());
    return false;
}

std::vector<models::Channel> ChannelsGroupModel::get_all_channels() const
{
    return channel_service_.get_all_channels();
}

std::vector<std::string> ChannelsGroupModel::get_available_group_names() const
{
    auto groups = channel_service_.get_all_channel_groups();
    std::vector<std::string> names;
    names.reserve(groups.size());

    for (const auto& group : groups)
    {
        names.push_back(group.name);
    }

    return names;
}

void ChannelsGroupModel::clear()
{
    channels_.clear();
    group_name_.clear();
    group_id_.clear();
}

bool ChannelsGroupModel::is_new_group() const
{
    return group_id_.empty();
}

// ========================================================================
// INTERNAL HELPERS
// ========================================================================

void ChannelsGroupModel::notify_groups_changed()
{
    if (on_groups_changed_callback_)
    {
        std::printf("[ChannelsGroupModel] Notifying parent of groups change\n");
        on_groups_changed_callback_();
    }
}

}  // namespace elda::views::channels_selector