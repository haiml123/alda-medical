#pragma once
#include "Channel.h"
#include <vector>
#include <string>

namespace elda::models {

    // Represents a group/montage of channels (similar to NeoRec profiles)
    struct ChannelsGroup {
        std::string name;                    // Group name (e.g., "10-20 System", "Custom Montage")
        std::vector<Channel> channels;       // Channels in this group
        std::string description;             // Optional description
        bool isDefault;                      // Whether this is a default/system profile

        ChannelsGroup() : isDefault(false) {}

        explicit ChannelsGroup(const std::string& name_)
            : name(name_), isDefault(false) {}

        // Add channel to group
        void addChannel(const Channel& channel) {
            channels.push_back(channel);
        }

        // Get number of selected channels
        size_t getSelectedCount() const {
            size_t count = 0;
            for (const auto& ch : channels) {
                if (ch.selected) ++count;
            }
            return count;
        }

        // Get all selected channels
        std::vector<Channel> getSelectedChannels() const {
            std::vector<Channel> selected;
            for (const auto& ch : channels) {
                if (ch.selected) {
                    selected.push_back(ch);
                }
            }
            return selected;
        }
    };

} // namespace elda