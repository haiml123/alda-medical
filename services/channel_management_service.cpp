#include "channel_management_service.h"

#include "secure_storage_service.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace elda::services
{

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

ChannelManagementService::ChannelManagementService()
{
    // Initialize storage services
    channel_storage_ = std::make_unique<elda::services::SecureConfigManager<std::vector<models::Channel>>>(
        "channels.dat",
        [](const std::vector<models::Channel>& channels) -> std::string
        {
            // Serialize channels to string
            std::ostringstream oss;
            oss << channels.size() << "\n";
            for (const auto& ch : channels)
            {
                oss << ch.get_id() << "|" << ch.name << "|" << ch.color << "|" << ch.selected << "|"
                    << ch.amplifier_channel << "|" << ch.signal_type << "|" << ch.sensor_gain << "|" << ch.sensor_offset
                    << "|" << ch.filtered << "|" << ch.high_pass_cutoff << "|" << ch.low_pass_cutoff << "\n";
            }
            return oss.str();
        },
        [](const std::string& data) -> std::vector<models::Channel>
        {
            // Deserialize string to channels
            std::vector<models::Channel> channels;
            std::istringstream iss(data);
            size_t count;
            iss >> count;
            iss.ignore();  // Skip newline

            for (size_t i = 0; i < count; ++i)
            {
                std::string line;
                if (!std::getline(iss, line))
                    break;

                std::istringstream line_stream(line);
                std::string token;
                std::vector<std::string> tokens;

                while (std::getline(line_stream, token, '|'))
                {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 11)
                {
                    models::Channel ch(tokens[0], tokens[1], tokens[2]);
                    ch.selected = (tokens[3] == "1");
                    ch.amplifier_channel = std::stoi(tokens[4]);
                    ch.signal_type = tokens[5];
                    ch.sensor_gain = std::stod(tokens[6]);
                    ch.sensor_offset = std::stod(tokens[7]);
                    ch.filtered = (tokens[8] == "1");
                    ch.high_pass_cutoff = std::stod(tokens[9]);
                    ch.low_pass_cutoff = std::stod(tokens[10]);
                    channels.push_back(ch);
                }
            }
            return channels;
        },
        true  // Enable backup
    );

    group_storage_ = std::make_unique<elda::services::SecureConfigManager<std::vector<models::ChannelsGroup>>>(
        "channel_groups.dat",
        [](const std::vector<models::ChannelsGroup>& groups) -> std::string
        {
            // Serialize channel groups
            std::ostringstream oss;
            oss << groups.size() << "\n";
            for (const auto& group : groups)
            {
                oss << group.id << "|" << group.name << "|" << group.description << "|" << group.is_default << "|"
                    << group.channel_ids.size() << "\n";

                // ✅ Serialize channel IDs only (not full objects)
                for (const auto& channel_id : group.channel_ids)
                {
                    oss << channel_id << "\n";
                }
            }
            return oss.str();
        },
        [](const std::string& data) -> std::vector<models::ChannelsGroup>
        {
            // Deserialize channel groups
            std::vector<models::ChannelsGroup> groups;
            std::istringstream iss(data);
            size_t group_count;
            iss >> group_count;
            iss.ignore();

            for (size_t i = 0; i < group_count; ++i)
            {
                std::string line;
                if (!std::getline(iss, line))
                    break;

                std::istringstream line_stream(line);
                std::string token;
                std::vector<std::string> tokens;

                while (std::getline(line_stream, token, '|'))
                {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 5)
                {
                    // ✅ Read: id, name, description, isDefault, channelCount
                    models::ChannelsGroup group(tokens[0], tokens[1]);
                    group.description = tokens[2];
                    group.is_default = (tokens[3] == "1");
                    size_t channel_count = std::stoul(tokens[4]);

                    // ✅ Read channel IDs (one per line)
                    for (size_t j = 0; j < channel_count; ++j)
                    {
                        std::string channel_id;
                        if (std::getline(iss, channel_id))
                        {
                            group.channel_ids.push_back(channel_id);
                        }
                    }

                    groups.push_back(group);
                }
            }
            return groups;
        },
        true  // Enable backup
    );

    active_group_storage_ = std::make_unique<elda::services::SecureConfigManager<models::ChannelsGroup>>(
        "active_channel_group.dat",
        [](const models::ChannelsGroup& group) -> std::string
        {
            // Serialize single group
            std::ostringstream oss;
            oss << group.id << "|" << group.name << "|" << group.description << "|" << group.is_default << "|"
                << group.channel_ids.size() << "\n";

            // ✅ Serialize channel IDs only
            for (const auto& channel_id : group.channel_ids)
            {
                oss << channel_id << "\n";
            }
            return oss.str();
        },
        [](const std::string& data) -> models::ChannelsGroup
        {
            // Deserialize single group
            std::istringstream iss(data);
            std::string line;
            std::getline(iss, line);

            std::istringstream line_stream(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(line_stream, token, '|'))
            {
                tokens.push_back(token);
            }

            models::ChannelsGroup group;
            if (tokens.size() >= 5)
            {
                group.id = tokens[0];
                group.name = tokens[1];
                group.description = tokens[2];
                group.is_default = (tokens[3] == "1");
                size_t channel_count = std::stoul(tokens[4]);

                for (size_t i = 0; i < channel_count; ++i)
                {
                    std::string channel_id;
                    if (std::getline(iss, channel_id))
                    {
                        group.channel_ids.push_back(channel_id);
                    }
                }
            }
            return group;
        },
        true  // Enable backup
    );

    // Load existing data
    load_from_storage();

    // Initialize 64 channels if none exist
    if (channels_.empty())
    {
        initialize_default_channels();
    }
}

ChannelManagementService& ChannelManagementService::get_instance()
{
    static ChannelManagementService instance;
    return instance;
}

// ============================================================================
// CHANNEL OPERATIONS
// ============================================================================

bool ChannelManagementService::create_channel(const models::Channel& channel)
{
    if (channel.get_id().empty())
        return false;
    if (channel_exists(channel.get_id()))
        return false;

    channels_.push_back(channel);
    save_to_storage();
    return true;
}

std::optional<models::Channel> ChannelManagementService::get_channel(const std::string& id) const
{
    auto it = find_channel_by_id(id);
    if (it != channels_.end())
    {
        return *it;
    }
    return std::nullopt;
}

const std::vector<models::Channel>& ChannelManagementService::get_all_channels() const
{
    return channels_;
}

bool ChannelManagementService::update_channel(const models::Channel& channel)
{
    auto it = find_channel_by_id(channel.id);
    if (it == channels_.end())
        return false;

    *it = channel;
    save_to_storage();
    return true;
}

bool ChannelManagementService::delete_channel(const std::string& id)
{
    auto it = find_channel_by_id(id);
    if (it == channels_.end())
        return false;

    channels_.erase(it);
    save_to_storage();
    return true;
}

bool ChannelManagementService::channel_exists(const std::string& id) const
{
    return find_channel_by_id(id) != channels_.end();
}

// ============================================================================
// CHANNEL GROUP OPERATIONS
// ============================================================================

bool ChannelManagementService::create_channel_group(const models::ChannelsGroup& group)
{
    if (group.name.empty())
        return false;
    if (group.id.empty())
        return false;
    if (channel_group_exists(group.id))
        return false;

    std::string error_message;
    if (!validate_channel_group(group, error_message))
        return false;

    channel_groups_.push_back(group);
    save_to_storage();
    return true;
}

std::optional<models::ChannelsGroup> ChannelManagementService::get_channel_group(const std::string& id) const
{
    auto it = find_group_by_id(id);
    if (it != channel_groups_.end())
    {
        return *it;
    }
    return std::nullopt;
}

std::optional<models::ChannelsGroup> ChannelManagementService::get_channel_group_by_name(const std::string& name) const
{
    auto it = find_group_by_name(name);
    if (it != channel_groups_.end())
    {
        return *it;
    }
    return std::nullopt;
}

std::vector<models::ChannelsGroup> ChannelManagementService::get_all_channel_groups() const
{
    return channel_groups_;
}

bool ChannelManagementService::update_channel_group(const models::ChannelsGroup& group)
{
    auto it = find_group_by_id(group.id);
    if (it == channel_groups_.end())
        return false;

    std::string error_message;
    if (!validate_channel_group(group, error_message))
        return false;

    *it = group;
    save_to_storage();
    return true;
}

int ChannelManagementService::delete_all_channel_groups()
{
    int deleted_count = 0;

    // Iterate backwards to safely erase while iterating
    for (auto it = channel_groups_.begin(); it != channel_groups_.end();)
    {
        if (!it->is_default)
        {
            it = channel_groups_.erase(it);  // erase returns iterator to next element
            deleted_count++;
        }
        else
        {
            ++it;  // Skip default groups
        }
    }

    if (deleted_count > 0)
    {
        // Clear active group if it was deleted
        // (Active group could have been one of the deleted groups)
        active_group_storage_->save_as_last_used(models::ChannelsGroup());

        save_to_storage();
    }

    return deleted_count;
}

bool ChannelManagementService::delete_channel_group(const std::string& id)
{
    auto it = find_group_by_id(id);
    if (it == channel_groups_.end())
        return false;

    // Don't allow deleting default groups
    if (it->is_default)
        return false;

    channel_groups_.erase(it);
    save_to_storage();
    return true;
}

bool ChannelManagementService::channel_group_exists(const std::string& id) const
{
    return find_group_by_id(id) != channel_groups_.end();
}

bool ChannelManagementService::channel_group_exists_by_name(const std::string& name) const
{
    return find_group_by_name(name) != channel_groups_.end();
}

// ============================================================================
// LAST USED / ACTIVE MANAGEMENT
// ============================================================================

bool ChannelManagementService::save_active_channel_group(const models::ChannelsGroup& group)
{
    return active_group_storage_->save_as_last_used(group);
}

std::optional<models::ChannelsGroup> ChannelManagementService::load_active_channel_group() const
{
    models::ChannelsGroup group;
    if (active_group_storage_->load_last_used(group))
    {
        return group;
    }
    return std::nullopt;
}

// ============================================================================
// VALIDATION & UTILITIES
// ============================================================================

bool ChannelManagementService::validate_channel_group(const models::ChannelsGroup& group,
                                                      std::string& error_message) const
{
    if (group.name.empty())
    {
        error_message = "Group name cannot be empty";
        return false;
    }

    // ✅ Check for duplicate channel IDs within the group
    std::set<std::string> unique_ids;
    for (const auto& channel_id : group.channel_ids)
    {
        if (!unique_ids.insert(channel_id).second)
        {
            error_message = "Duplicate channel ID found: " + channel_id;
            return false;
        }
    }

    return true;
}

std::vector<models::ChannelsGroup> ChannelManagementService::get_default_channel_groups() const
{
    std::vector<models::ChannelsGroup> defaults;

    // Return only groups marked as default
    for (const auto& group : channel_groups_)
    {
        if (group.is_default)
        {
            defaults.push_back(group);
        }
    }

    return defaults;
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================

void ChannelManagementService::load_from_storage()
{
    // Load channels
    std::vector<models::Channel> loaded_channels;
    if (channel_storage_->load_last_used(loaded_channels))
    {
        channels_ = loaded_channels;
    }

    // Load channel groups
    std::vector<models::ChannelsGroup> loaded_groups;
    if (group_storage_->load_last_used(loaded_groups))
    {
        channel_groups_ = loaded_groups;
    }
}

void ChannelManagementService::save_to_storage()
{
    channel_storage_->save_as_last_used(channels_);
    group_storage_->save_as_last_used(channel_groups_);
}

std::vector<models::Channel>::iterator ChannelManagementService::find_channel_by_id(const std::string& id)
{
    return std::find_if(channels_.begin(),
                        channels_.end(),
                        [&id](const models::Channel& ch)
                        {
                            return ch.get_id() == id;
                        });
}

std::vector<models::Channel>::const_iterator ChannelManagementService::find_channel_by_id(const std::string& id) const
{
    return std::find_if(channels_.begin(),
                        channels_.end(),
                        [&id](const models::Channel& ch)
                        {
                            return ch.get_id() == id;
                        });
}

std::vector<models::ChannelsGroup>::iterator ChannelManagementService::find_group_by_id(const std::string& id)
{
    return std::find_if(channel_groups_.begin(),
                        channel_groups_.end(),
                        [&id](const models::ChannelsGroup& g)
                        {
                            return g.id == id;
                        });
}

std::vector<models::ChannelsGroup>::const_iterator
ChannelManagementService::find_group_by_id(const std::string& id) const
{
    return std::find_if(channel_groups_.begin(),
                        channel_groups_.end(),
                        [&id](const models::ChannelsGroup& g)
                        {
                            return g.id == id;
                        });
}

std::vector<models::ChannelsGroup>::iterator ChannelManagementService::find_group_by_name(const std::string& name)
{
    return std::find_if(channel_groups_.begin(),
                        channel_groups_.end(),
                        [&name](const models::ChannelsGroup& g)
                        {
                            return g.name == name;
                        });
}

std::vector<models::ChannelsGroup>::const_iterator
ChannelManagementService::find_group_by_name(const std::string& name) const
{
    return std::find_if(channel_groups_.begin(),
                        channel_groups_.end(),
                        [&name](const models::ChannelsGroup& g)
                        {
                            return g.name == name;
                        });
}

bool ChannelManagementService::initialize_default_channels()
{
    if (!channels_.empty())
    {
        return true;  // Already initialized
    }

    // EEG standard channel names (10-20 system)
    const char* channel_names[64] = {"Fp1", "Fp2", "F7",  "F3",   "Fz",  "F4",   "F8",   "FC5",  "FC1", "FC2", "FC6",
                                     "T7",  "T8",  "TP9", "TP10", "C3",  "Cz",   "C4",   "CP5",  "CP1", "CP2", "CP6",
                                     "P7",  "P3",  "Pz",  "P4",   "P8",  "PO9",  "PO10", "O1",   "Oz",  "O2",  "AF3",
                                     "AF4", "F5",  "F1",  "F2",   "F6",  "FT7",  "FC3",  "FC4",  "FT8", "C5",  "C1",
                                     "C2",  "C6",  "TP7", "CP3",  "CPz", "CP4",  "TP8",  "P5",   "P1",  "P2",  "P6",
                                     "PO7", "PO3", "POz", "PO4",  "PO8", "Ch57", "Ch58", "Ch59", "Ch60"};

    const char* colors[] = {
        "#FF6B6B", "#4ECDC4", "#45B7D1", "#FFA07A", "#98D8C8", "#F7DC6F", "#BB8FCE", "#85C1E2", "#F8B739", "#52C9B1"};

    channels_.clear();
    channels_.reserve(64);

    for (int i = 0; i < 64; ++i)
    {
        char id[16];
        std::snprintf(id, sizeof(id), "ch_%d", i);
        models::Channel channel(id, channel_names[i], colors[i % 10]);
        channels_.push_back(channel);
    }

    save_to_storage();

    // // ✅ Create default group with all channel IDs
    // models::ChannelsGroup default_group("default_group", "Standard 64-Channel");
    // for (const auto& channel : channels_) {
    //     default_group.addChannelId(channel.id);
    // }
    // default_group.is_default = true;
    //
    // create_channel_group(default_group);
    // save_active_channel_group(default_group);

    return true;
}

}  // namespace elda::services
