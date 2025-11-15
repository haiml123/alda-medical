#pragma once
#include "base_model.h"
#include <vector>
#include <string>
#include <set>

namespace elda::models {

    struct ChannelsGroup : public BaseModel {
        std::string name;
        std::vector<std::string> channel_ids;
        std::string description;
        bool is_default;

        ChannelsGroup() : BaseModel(), is_default(false) {}

        explicit ChannelsGroup(const std::string& name_)
            : BaseModel(), name(name_), is_default(false) {}

        ChannelsGroup(const std::string& id_, const std::string& name_)
            : BaseModel(id_), name(name_), is_default(false) {}

        void add_channel_id(const std::string& channel_id) {
            channel_ids.push_back(channel_id);
            on_update();
        }

        void remove_channel_id(const std::string& channel_id) {
            channel_ids.erase(
                std::remove(channel_ids.begin(), channel_ids.end(), channel_id),
                channel_ids.end()
            );
            on_update();
        }

        bool has_channel(const std::string& channel_id) const {
            return std::find(channel_ids.begin(), channel_ids.end(), channel_id)
                   != channel_ids.end();
        }

        size_t get_channel_count() const {
            return channel_ids.size();
        }
    };

}
