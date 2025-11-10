#include "channels_group_model.h"
#include <algorithm>

namespace elda::channels_group {

    ChannelsGroupModel::ChannelsGroupModel(elda::AppStateManager& stateManager)
        : MVPBaseModel(stateManager)
        , channelService_(services::ChannelManagementService::GetInstance())
        , onGroupsChangedCallback_(nullptr) {
    }

    // ========================================================================
    // DATA ACCESS
    // ========================================================================

    int ChannelsGroupModel::GetSelectedCount() const {
        return std::count_if(channels_.begin(), channels_.end(),
                           [](const models::Channel& ch) { return ch.selected; });
    }

    // ========================================================================
    // DATA MODIFICATION
    // ========================================================================

    void ChannelsGroupModel::SetChannels(const std::vector<models::Channel>& channels) {
        channels_ = channels;
    }

    void ChannelsGroupModel::SelectChannel(size_t index, bool selected) {
        if (index < channels_.size()) {
            channels_[index].selected = selected;
        }
    }

    void ChannelsGroupModel::SelectAllChannels(bool selected) {
        for (auto& ch : channels_) {
            ch.selected = selected;
        }
    }

    // ========================================================================
    // VALIDATION
    // ========================================================================

    bool ChannelsGroupModel::CanConfirm(std::string& errorMessage) const {
        if (groupName_.empty()) {
            errorMessage = "Group name cannot be empty";
            return false;
        }

        if (GetSelectedCount() == 0) {
            errorMessage = "At least one channel must be selected";
            return false;
        }

        return true;
    }

    // ========================================================================
    // BUSINESS LOGIC
    // ========================================================================

    bool ChannelsGroupModel::LoadChannelGroupById(const std::string& id) {
        auto group = channelService_.GetChannelGroup(id);
        if (group.has_value()) {
            groupId_ = group->id;  // ✅ CRITICAL: Store the ID from BaseModel
            groupName_ = group->name;

            channels_ = channelService_.GetAllChannels();

            // Mark channels that are in the group as selected
            for (auto& channel : channels_) {
                channel.selected = std::find(group->channelIds.begin(),
                                            group->channelIds.end(),
                                            channel.GetId()) != group->channelIds.end();
            }

            return true;
        }
        return false;
    }

    bool ChannelsGroupModel::LoadActiveChannelGroup() {
        auto group = channelService_.LoadActiveChannelGroup();
        if (group.has_value()) {
            groupId_ = group->id;
            groupName_ = group->name;

            channels_ = channelService_.GetAllChannels();

            // Mark channels that are in the group as selected
            for (auto& channel : channels_) {
                channel.selected = std::find(group->channelIds.begin(),
                                            group->channelIds.end(),
                                            channel.GetId()) != group->channelIds.end();
            }

            return true;
        }
        return false;
    }

    bool ChannelsGroupModel::SaveChannelGroup() {
        // TODO: This should go through AppStateManager for:
        // - State validation
        // - Audit logging
        // - Observer notifications
        // - Medical device compliance
        // See APPSTATE_INTEGRATION.cpp for implementation options

        std::vector<std::string> selectedChannelIds;
        for (const auto& channel : channels_) {
            if (channel.selected) {
                selectedChannelIds.push_back(channel.id);
            }
        }

        bool success = false;

        if (groupId_.empty()) {
            // CREATE: New group
            models::ChannelsGroup group(groupName_);
            group.channelIds = selectedChannelIds;
            group.OnCreate();

            if (channelService_.CreateChannelGroup(group)) {
                // Remember the ID for future operations
                groupId_ = group.id;
                success = true;

                std::printf("[ChannelsGroupModel] ✓ Created group: %s (ID: %s)\n",
                           groupName_.c_str(), groupId_.c_str());
            } else {
                std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to create group: %s\n",
                            groupName_.c_str());
            }

        } else {
            // UPDATE: Existing group - preserve ID, allow name change
            models::ChannelsGroup group(groupId_, groupName_);
            group.channelIds = selectedChannelIds;
            group.OnUpdate();

            if (channelService_.UpdateChannelGroup(group)) {
                success = true;

                std::printf("[ChannelsGroupModel] ✓ Updated group: %s (ID: %s)\n",
                           groupName_.c_str(), groupId_.c_str());
            } else {
                std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to update group: %s\n",
                            groupName_.c_str());
            }
        }

        if (success) {
            // Save as active group
            models::ChannelsGroup activeGroup(groupId_, groupName_);
            activeGroup.channelIds = selectedChannelIds;
            channelService_.SaveActiveChannelGroup(activeGroup);

            NotifyGroupsChanged();
        }

        return success;
    }

    bool ChannelsGroupModel::DeleteChannelGroup() {
        if (groupId_.empty()) {
            return false;  // Can't delete a group that doesn't exist yet
        }

        // TODO: This should go through AppStateManager for:
        // - State validation
        // - Audit logging
        // - Observer notifications
        // - Medical device compliance
        // See APPSTATE_INTEGRATION.cpp for implementation options

        if (channelService_.DeleteChannelGroup(groupId_)) {
            std::printf("[ChannelsGroupModel] ✓ Deleted group: %s (ID: %s)\n",
                       groupName_.c_str(), groupId_.c_str());

            NotifyGroupsChanged();

            return true;
        }

        std::fprintf(stderr, "[ChannelsGroupModel] ✗ Failed to delete group: %s\n",
                    groupName_.c_str());
        return false;
    }

    std::vector<models::Channel> ChannelsGroupModel::GetAllChannels() const {
        return channelService_.GetAllChannels();
    }

    std::vector<std::string> ChannelsGroupModel::GetAvailableGroupNames() const {
        auto groups = channelService_.GetAllChannelGroups();
        std::vector<std::string> names;
        names.reserve(groups.size());

        for (const auto& group : groups) {
            names.push_back(group.name);
        }

        return names;
    }

    void ChannelsGroupModel::Clear() {
        channels_.clear();
        groupName_.clear();
        groupId_.clear();
    }

    bool ChannelsGroupModel::IsNewGroup() const {
        return groupId_.empty();
    }

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================

    void ChannelsGroupModel::NotifyGroupsChanged() {
        if (onGroupsChangedCallback_) {
            std::printf("[ChannelsGroupModel] Notifying parent of groups change\n");
            onGroupsChangedCallback_();
        }
    }

} // namespace elda::channels_group