#include "channels_group_model.h"
#include <algorithm>

namespace elda::channels_group {

    ChannelsGroupModel::ChannelsGroupModel()
        : channelService_(services::ChannelManagementService::GetInstance()) {
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

    bool ChannelsGroupModel::LoadChannelGroup(const std::string& groupName) {
        auto group = channelService_.GetChannelGroup(groupName);
        if (group.has_value()) {
            groupId_ = group->id;  // ✅ CRITICAL: Store the ID from BaseModel
            groupName_ = group->name;
            channels_ = group->channels;
            return true;
        }
        return false;
    }

    bool ChannelsGroupModel::LoadActiveChannelGroup() {
        auto group = channelService_.LoadActiveChannelGroup();
        if (group.has_value()) {
            groupId_ = group->id;  // ✅ CRITICAL: Store the ID from BaseModel
            groupName_ = group->name;
            channels_ = group->channels;
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

        // ✅ CRITICAL FIX: Use groupId_ to determine create vs update
        if (groupId_.empty()) {
            // CREATE: New group
            models::ChannelsGroup group(groupName_);
            group.channels = channels_;

            // ✅ Initialize BaseModel fields
            group.OnCreate();

            if (!channelService_.CreateChannelGroup(group)) {
                return false;
            }

            // Remember the ID for future operations
            groupId_ = group.id;

        } else {
            // UPDATE: Existing group - preserve ID, allow name change
            models::ChannelsGroup group(groupId_, groupName_);
            group.channels = channels_;

            // ✅ Update BaseModel timestamp
            group.OnUpdate();

            if (!channelService_.UpdateChannelGroup(group)) {
                return false;
            }
        }

        // Save as active group
        models::ChannelsGroup activeGroup(groupId_, groupName_);
        activeGroup.channels = channels_;
        return channelService_.SaveActiveChannelGroup(activeGroup);
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

        // ✅ CRITICAL: Delete using ID, not name!
        return channelService_.DeleteChannelGroup(groupId_);
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
        groupId_.clear();  // ✅ CRITICAL: Also clear the ID
    }

    bool ChannelsGroupModel::IsNewGroup() const {
        // ✅ UPDATED: A group is "new" if it has no ID yet
        return groupId_.empty();
    }

} // namespace elda::channels_group