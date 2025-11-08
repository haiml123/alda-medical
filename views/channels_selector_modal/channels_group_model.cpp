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
            channels_ = group->channels;
            groupName_ = group->name;
            return true;
        }
        return false;
    }

    bool ChannelsGroupModel::LoadActiveChannelGroup() {
        auto group = channelService_.LoadActiveChannelGroup();
        if (group.has_value()) {
            channels_ = group->channels;
            groupName_ = group->name;
            return true;
        }
        return false;
    }

    bool ChannelsGroupModel::SaveChannelGroup() {
        models::ChannelsGroup group(groupName_);
        group.channels = channels_;

        // Check if group exists - update or create
        if (channelService_.ChannelGroupExists(groupName_)) {
            if (!channelService_.UpdateChannelGroup(group)) {
                return false;
            }
        } else {
            if (!channelService_.CreateChannelGroup(group)) {
                return false;
            }
        }

        // Save as active group
        return channelService_.SaveActiveChannelGroup(group);
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
    }

} // namespace elda::channels_group