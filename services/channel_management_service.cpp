#include "channel_management_service.h"
#include "secure_storage_service.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <set>

namespace elda::services {

    // ============================================================================
    // CONSTRUCTOR & INITIALIZATION
    // ============================================================================

    ChannelManagementService::ChannelManagementService() {
        // Initialize storage services
        channelStorage_ = std::make_unique<elda::services::SecureConfigManager<std::vector<models::Channel>>>(
            "channels.dat",
            [](const std::vector<models::Channel>& channels) -> std::string {
                // Serialize channels to string
                std::ostringstream oss;
                oss << channels.size() << "\n";
                for (const auto& ch : channels) {
                    oss << ch.GetId() << "|"
                        << ch.name << "|"
                        << ch.color << "|"
                        << ch.selected << "|"
                        << ch.amplifierChannel << "|"
                        << ch.signalType << "|"
                        << ch.sensorGain << "|"
                        << ch.sensorOffset << "|"
                        << ch.filtered << "|"
                        << ch.highPassCutoff << "|"
                        << ch.lowPassCutoff << "\n";
                }
                return oss.str();
            },
            [](const std::string& data) -> std::vector<models::Channel> {
                // Deserialize string to channels
                std::vector<models::Channel> channels;
                std::istringstream iss(data);
                size_t count;
                iss >> count;
                iss.ignore(); // Skip newline

                for (size_t i = 0; i < count; ++i) {
                    std::string line;
                    if (!std::getline(iss, line)) break;

                    std::istringstream lineStream(line);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(lineStream, token, '|')) {
                        tokens.push_back(token);
                    }

                    if (tokens.size() >= 11) {
                        models::Channel ch(tokens[0], tokens[1], tokens[2]);
                        ch.selected = (tokens[3] == "1");
                        ch.amplifierChannel = std::stoi(tokens[4]);
                        ch.signalType = tokens[5];
                        ch.sensorGain = std::stod(tokens[6]);
                        ch.sensorOffset = std::stod(tokens[7]);
                        ch.filtered = (tokens[8] == "1");
                        ch.highPassCutoff = std::stod(tokens[9]);
                        ch.lowPassCutoff = std::stod(tokens[10]);
                        channels.push_back(ch);
                    }
                }
                return channels;
            },
            true // Enable backup
        );

        groupStorage_ = std::make_unique<elda::services::SecureConfigManager<std::vector<models::ChannelsGroup>>>(
            "channel_groups.dat",
            [](const std::vector<models::ChannelsGroup>& groups) -> std::string {
                // Serialize channel groups
                std::ostringstream oss;
                oss << groups.size() << "\n";
                for (const auto& group : groups) {
                    oss << group.id << "|"
                        << group.name << "|"
                        << group.description << "|"
                        << group.isDefault << "|"
                        << group.channelIds.size() << "\n";

                    // ✅ Serialize channel IDs only (not full objects)
                    for (const auto& channelId : group.channelIds) {
                        oss << channelId << "\n";
                    }
                }
                return oss.str();
            },
            [](const std::string& data) -> std::vector<models::ChannelsGroup> {
                // Deserialize channel groups
                std::vector<models::ChannelsGroup> groups;
                std::istringstream iss(data);
                size_t groupCount;
                iss >> groupCount;
                iss.ignore();

                for (size_t i = 0; i < groupCount; ++i) {
                    std::string line;
                    if (!std::getline(iss, line)) break;

                    std::istringstream lineStream(line);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(lineStream, token, '|')) {
                        tokens.push_back(token);
                    }

                    if (tokens.size() >= 5) {
                        // ✅ Read: id, name, description, isDefault, channelCount
                        models::ChannelsGroup group(tokens[0], tokens[1]);
                        group.description = tokens[2];
                        group.isDefault = (tokens[3] == "1");
                        size_t channelCount = std::stoul(tokens[4]);

                        // ✅ Read channel IDs (one per line)
                        for (size_t j = 0; j < channelCount; ++j) {
                            std::string channelId;
                            if (std::getline(iss, channelId)) {
                                group.channelIds.push_back(channelId);
                            }
                        }

                        groups.push_back(group);
                    }
                }
                return groups;
            },
            true // Enable backup
        );

        activeGroupStorage_ = std::make_unique<elda::services::SecureConfigManager<models::ChannelsGroup>>(
            "active_channel_group.dat",
            [](const models::ChannelsGroup& group) -> std::string {
                // Serialize single group
                std::ostringstream oss;
                oss << group.id << "|"
                    << group.name << "|"
                    << group.description << "|"
                    << group.isDefault << "|"
                    << group.channelIds.size() << "\n";

                // ✅ Serialize channel IDs only
                for (const auto& channelId : group.channelIds) {
                    oss << channelId << "\n";
                }
                return oss.str();
            },
            [](const std::string& data) -> models::ChannelsGroup {
                // Deserialize single group
                std::istringstream iss(data);
                std::string line;
                std::getline(iss, line);

                std::istringstream lineStream(line);
                std::string token;
                std::vector<std::string> tokens;

                while (std::getline(lineStream, token, '|')) {
                    tokens.push_back(token);
                }

                models::ChannelsGroup group;
                if (tokens.size() >= 5) {
                    // ✅ Read: id, name, description, isDefault, channelCount
                    group.id = tokens[0];
                    group.name = tokens[1];
                    group.description = tokens[2];
                    group.isDefault = (tokens[3] == "1");
                    size_t channelCount = std::stoul(tokens[4]);

                    // ✅ Read channel IDs (one per line)
                    for (size_t i = 0; i < channelCount; ++i) {
                        std::string channelId;
                        if (std::getline(iss, channelId)) {
                            group.channelIds.push_back(channelId);
                        }
                    }
                }
                return group;
            },
            true // Enable backup
        );

        // Load existing data
        LoadFromStorage();

        // ✅ Initialize 64 channels if none exist
        if (channels_.empty()) {
            InitializeDefaultChannels();
        }
    }

    ChannelManagementService& ChannelManagementService::GetInstance() {
        static ChannelManagementService instance;
        return instance;
    }

    // ============================================================================
    // CHANNEL OPERATIONS
    // ============================================================================

    bool ChannelManagementService::CreateChannel(const models::Channel& channel) {
        if (channel.GetId().empty()) return false;
        if (ChannelExists(channel.GetId())) return false;

        channels_.push_back(channel);
        SaveToStorage();
        return true;
    }

    std::optional<models::Channel> ChannelManagementService::GetChannel(const std::string& id) const {
        auto it = FindChannelById(id);
        if (it != channels_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    const std::vector<models::Channel>& ChannelManagementService::GetAllChannels() const {
        return channels_;
    }

    bool ChannelManagementService::UpdateChannel(const models::Channel& channel) {
        auto it = FindChannelById(channel.id);
        if (it == channels_.end()) return false;

        *it = channel;
        SaveToStorage();
        return true;
    }

    bool ChannelManagementService::DeleteChannel(const std::string& id) {
        auto it = FindChannelById(id);
        if (it == channels_.end()) return false;

        channels_.erase(it);
        SaveToStorage();
        return true;
    }

    bool ChannelManagementService::ChannelExists(const std::string& id) const {
        return FindChannelById(id) != channels_.end();
    }

    // ============================================================================
    // CHANNEL GROUP OPERATIONS
    // ============================================================================

    bool ChannelManagementService::CreateChannelGroup(const models::ChannelsGroup& group) {
        if (group.name.empty()) return false;
        if (group.id.empty()) return false;
        if (ChannelGroupExists(group.id)) return false;

        std::string errorMessage;
        if (!ValidateChannelGroup(group, errorMessage)) return false;

        channelGroups_.push_back(group);
        SaveToStorage();
        return true;
    }

    std::optional<models::ChannelsGroup> ChannelManagementService::GetChannelGroup(const std::string& id) const {
        auto it = FindGroupById(id);
        if (it != channelGroups_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::optional<models::ChannelsGroup> ChannelManagementService::GetChannelGroupByName(const std::string& name) const {
        auto it = FindGroupByName(name);
        if (it != channelGroups_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::vector<models::ChannelsGroup> ChannelManagementService::GetAllChannelGroups() const {
        return channelGroups_;
    }

    bool ChannelManagementService::UpdateChannelGroup(const models::ChannelsGroup& group) {
        auto it = FindGroupById(group.id);
        if (it == channelGroups_.end()) return false;

        std::string errorMessage;
        if (!ValidateChannelGroup(group, errorMessage)) return false;

        *it = group;
        SaveToStorage();
        return true;
    }

    int ChannelManagementService::DeleteAllChannelGroups() {
        int deletedCount = 0;

        // Iterate backwards to safely erase while iterating
        for (auto it = channelGroups_.begin(); it != channelGroups_.end(); ) {
            if (!it->isDefault) {
                it = channelGroups_.erase(it);  // erase returns iterator to next element
                deletedCount++;
            } else {
                ++it;  // Skip default groups
            }
        }

        if (deletedCount > 0) {
            // Clear active group if it was deleted
            // (Active group could have been one of the deleted groups)
            activeGroupStorage_->SaveAsLastUsed(models::ChannelsGroup());

            SaveToStorage();
        }

        return deletedCount;
    }

    bool ChannelManagementService::DeleteChannelGroup(const std::string& id) {
        auto it = FindGroupById(id);
        if (it == channelGroups_.end()) return false;

        // Don't allow deleting default groups
        if (it->isDefault) return false;

        channelGroups_.erase(it);
        SaveToStorage();
        return true;
    }

    bool ChannelManagementService::ChannelGroupExists(const std::string& id) const {
        return FindGroupById(id) != channelGroups_.end();
    }

    bool ChannelManagementService::ChannelGroupExistsByName(const std::string& name) const {
        return FindGroupByName(name) != channelGroups_.end();
    }

    // ============================================================================
    // LAST USED / ACTIVE MANAGEMENT
    // ============================================================================

    bool ChannelManagementService::SaveActiveChannelGroup(const models::ChannelsGroup& group) {
        return activeGroupStorage_->SaveAsLastUsed(group);
    }

    std::optional<models::ChannelsGroup> ChannelManagementService::LoadActiveChannelGroup() const {
        models::ChannelsGroup group;
        if (activeGroupStorage_->LoadLastUsed(group)) {
            return group;
        }
        return std::nullopt;
    }

    // ============================================================================
    // VALIDATION & UTILITIES
    // ============================================================================

    bool ChannelManagementService::ValidateChannelGroup(const models::ChannelsGroup& group, std::string& errorMessage) const {
        if (group.name.empty()) {
            errorMessage = "Group name cannot be empty";
            return false;
        }

        // ✅ Check for duplicate channel IDs within the group
        std::set<std::string> uniqueIds;
        for (const auto& channelId : group.channelIds) {
            if (!uniqueIds.insert(channelId).second) {
                errorMessage = "Duplicate channel ID found: " + channelId;
                return false;
            }
        }

        return true;
    }

    std::vector<models::ChannelsGroup> ChannelManagementService::GetDefaultChannelGroups() const {
        std::vector<models::ChannelsGroup> defaults;

        // Return only groups marked as default
        for (const auto& group : channelGroups_) {
            if (group.isDefault) {
                defaults.push_back(group);
            }
        }

        return defaults;
    }

    // ============================================================================
    // PRIVATE HELPER METHODS
    // ============================================================================

    void ChannelManagementService::LoadFromStorage() {
        // Load channels
        std::vector<models::Channel> loadedChannels;
        if (channelStorage_->LoadLastUsed(loadedChannels)) {
            channels_ = loadedChannels;
        }

        // Load channel groups
        std::vector<models::ChannelsGroup> loadedGroups;
        if (groupStorage_->LoadLastUsed(loadedGroups)) {
            channelGroups_ = loadedGroups;
        }
    }

    void ChannelManagementService::SaveToStorage() {
        channelStorage_->SaveAsLastUsed(channels_);
        groupStorage_->SaveAsLastUsed(channelGroups_);
    }

    std::vector<models::Channel>::iterator ChannelManagementService::FindChannelById(const std::string& id) {
        return std::find_if(channels_.begin(), channels_.end(),
            [&id](const models::Channel& ch) { return ch.GetId() == id; });
    }

    std::vector<models::Channel>::const_iterator ChannelManagementService::FindChannelById(const std::string& id) const {
        return std::find_if(channels_.begin(), channels_.end(),
            [&id](const models::Channel& ch) { return ch.GetId() == id; });
    }

    std::vector<models::ChannelsGroup>::iterator ChannelManagementService::FindGroupById(const std::string& id) {
        return std::find_if(channelGroups_.begin(), channelGroups_.end(),
            [&id](const models::ChannelsGroup& g) { return g.id == id; });
    }

    std::vector<models::ChannelsGroup>::const_iterator ChannelManagementService::FindGroupById(const std::string& id) const {
        return std::find_if(channelGroups_.begin(), channelGroups_.end(),
            [&id](const models::ChannelsGroup& g) { return g.id == id; });
    }

    std::vector<models::ChannelsGroup>::iterator ChannelManagementService::FindGroupByName(const std::string& name) {
        return std::find_if(channelGroups_.begin(), channelGroups_.end(),
            [&name](const models::ChannelsGroup& g) { return g.name == name; });
    }

    std::vector<models::ChannelsGroup>::const_iterator ChannelManagementService::FindGroupByName(const std::string& name) const {
        return std::find_if(channelGroups_.begin(), channelGroups_.end(),
            [&name](const models::ChannelsGroup& g) { return g.name == name; });
    }

    bool ChannelManagementService::InitializeDefaultChannels() {
        if (!channels_.empty()) {
            return true; // Already initialized
        }

        // EEG standard channel names (10-20 system)
        const char* channelNames[64] = {
            "Fp1", "Fp2", "F7", "F3", "Fz", "F4", "F8",
            "FC5", "FC1", "FC2", "FC6",
            "T7", "T8", "TP9", "TP10",
            "C3", "Cz", "C4", "CP5", "CP1", "CP2", "CP6",
            "P7", "P3", "Pz", "P4", "P8", "PO9", "PO10",
            "O1", "Oz", "O2",
            "AF3", "AF4", "F5", "F1", "F2", "F6",
            "FT7", "FC3", "FC4", "FT8",
            "C5", "C1", "C2", "C6",
            "TP7", "CP3", "CPz", "CP4", "TP8",
            "P5", "P1", "P2", "P6",
            "PO7", "PO3", "POz", "PO4", "PO8",
            "Ch57", "Ch58", "Ch59", "Ch60"
        };

        const char* colors[] = {
            "#FF6B6B", "#4ECDC4", "#45B7D1", "#FFA07A", "#98D8C8",
            "#F7DC6F", "#BB8FCE", "#85C1E2", "#F8B739", "#52C9B1"
        };

        channels_.clear();
        channels_.reserve(64);

        for (int i = 0; i < 64; ++i) {
            char id[16];
            std::snprintf(id, sizeof(id), "ch_%d", i);
            models::Channel channel(id, channelNames[i], colors[i % 10]);
            channels_.push_back(channel);
        }

        SaveToStorage();

        // // ✅ Create default group with all channel IDs
        // models::ChannelsGroup defaultGroup("default_group", "Standard 64-Channel");
        // for (const auto& channel : channels_) {
        //     defaultGroup.addChannelId(channel.id);
        // }
        // defaultGroup.isDefault = true;
        //
        // CreateChannelGroup(defaultGroup);
        // SaveActiveChannelGroup(defaultGroup);

        return true;
    }

} // namespace elda::services