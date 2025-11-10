#pragma once
#include "base_model.h"
#include <vector>
#include <string>
#include <set>

namespace elda::models {

    struct ChannelsGroup : public BaseModel {
        std::string name;
        std::vector<std::string> channelIds;  // Store IDs only!
        std::string description;
        bool isDefault;

        ChannelsGroup() : BaseModel(), isDefault(false) {}

        explicit ChannelsGroup(const std::string& name_)
            : BaseModel(), name(name_), isDefault(false) {}

        ChannelsGroup(const std::string& id_, const std::string& name_)
            : BaseModel(id_), name(name_), isDefault(false) {}

        void addChannelId(const std::string& channelId) {
            channelIds.push_back(channelId);
            OnUpdate();
        }

        void removeChannelId(const std::string& channelId) {
            channelIds.erase(
                std::remove(channelIds.begin(), channelIds.end(), channelId),
                channelIds.end()
            );
            OnUpdate();
        }

        bool hasChannel(const std::string& channelId) const {
            return std::find(channelIds.begin(), channelIds.end(), channelId)
                   != channelIds.end();
        }

        size_t getChannelCount() const {
            return channelIds.size();
        }
    };

} // namespace elda::models