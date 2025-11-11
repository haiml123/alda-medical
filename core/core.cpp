#include "core/core.h"
#include "models/channel.h"
#include "services/channel_management_service.h"

// AppState constructor implementation
AppState::AppState() {
    chNames.reserve(CHANNELS);
    for (int i = 0; i < CHANNELS; ++i) {
        char b[16];
        std::snprintf(b, sizeof(b), "Ch%02d", i + 1);
        chNames.emplace_back(b);
    }

    // Initialize available channels
    InitializeChannels();
    InitializeGroupChannels();
}

// Initialize channels with default colors
void AppState::InitializeChannels() {
    auto& service = elda::services::ChannelManagementService::GetInstance();
    availableChannels = &service.GetAllChannels(); // mirror into AppState
}

void AppState::InitializeGroupChannels() {
    auto& service = elda::services::ChannelManagementService::GetInstance();
    availableGroups = service.GetAllChannelGroups();

    if (availableGroups.empty()) {
        std::printf("[AppState] No channel groups found, creating default...\n");
        CreateDefaultGroups();
        availableGroups = service.GetAllChannelGroups();
    }
}

void AppState::CreateDefaultGroups() {
    auto& service = elda::services::ChannelManagementService::GetInstance();
    // Create the group
    elda::models::ChannelsGroup allChannels("All Channels");
    allChannels.description = "All 64 channels";
    allChannels.isDefault = true;
    allChannels.OnCreate();  // ✅ Generate ID and timestamps

    // Add all available channels as selected
    for (const auto& ch : *availableChannels) {
        auto channel = ch;  // Make a copy
        channel.selected = true;
        allChannels.addChannelId(channel.id);
    }

    // Save to database
    if (service.CreateChannelGroup(allChannels)) {
        printf("Group %s has %zu channels\n", allChannels.id.c_str(), allChannels.getChannelCount());
    } else {
        std::fprintf(stderr, "[AppState] ✗ Failed to create 'All Channels' group\n");
    }
}