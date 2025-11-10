#pragma once
#include "channel.h"
#include "base_model.h"
#include <vector>
#include <string>

#include "base_model.h"

namespace elda::models {

    // Option 1: Extend BaseModel (Recommended)
    struct ChannelsGroup : public BaseModel {
        std::string name;
        std::vector<Channel> channels;
        std::string description;
        bool isDefault;

        ChannelsGroup() : BaseModel(), isDefault(false) {}

        explicit ChannelsGroup(const std::string& name_)
            : BaseModel(), name(name_), isDefault(false) {}

        ChannelsGroup(const std::string& id_, const std::string& name_)
            : BaseModel(id_), name(name_), isDefault(false) {}

        void addChannel(const Channel& channel) {
            channels.push_back(channel);
            OnUpdate();
        }

        size_t getSelectedCount() const {
            size_t count = 0;
            for (const auto& ch : channels) {
                if (ch.selected) ++count;
            }
            return count;
        }

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

    // Option 2: Create derived class from ChannelsGroup
    struct CustomChannelsGroup : public ChannelsGroup {
        std::string author;
        bool isShared;

        CustomChannelsGroup() : ChannelsGroup(), isShared(false) {}

        explicit CustomChannelsGroup(const std::string& name_)
            : ChannelsGroup(name_), isShared(false) {}
    };

    // Option 3: Specialized group types
    struct PredefinedChannelsGroup : public ChannelsGroup {
        std::string standard;  // e.g., "10-20", "10-10"

        PredefinedChannelsGroup() : ChannelsGroup() {
            isDefault = true;
        }
    };

} // namespace elda::models