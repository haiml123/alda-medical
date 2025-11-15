#include "core/core.h"
#include "models/channel.h"
#include "services/channel_management_service.h"

// AppState constructor implementation
AppState::AppState() {
    ch_names.reserve(CHANNELS);
    for (int i = 0; i < CHANNELS; ++i) {
        char b[16];
        std::snprintf(b, sizeof(b), "Ch%02d", i + 1);
        ch_names.emplace_back(b);
    }

    // Initialize available channels
    initialize_channels();
    initialize_group_channels();
}

// Initialize channels with default colors
void AppState::initialize_channels() {
    auto& service = elda::services::ChannelManagementService::get_instance();
    available_channels = &service.get_all_channels(); // mirror into AppState
}

void AppState::initialize_group_channels() {
    auto& service = elda::services::ChannelManagementService::get_instance();
    available_groups = service.get_all_channel_groups();

    if (available_groups.empty()) {
        std::printf("[AppState] No channel groups found, creating default...\n");
        create_default_groups();
        available_groups = service.get_all_channel_groups();
    }
}

void AppState::create_default_groups() {
    auto& service = elda::services::ChannelManagementService::get_instance();
    // Create the group
    elda::models::ChannelsGroup all_channels("All Channels");
    all_channels.description = "All 64 channels";
    all_channels.is_default = true;
    all_channels.on_create();  // ✅ Generate ID and timestamps

    // Add all available channels as selected
    for (const auto& ch : *available_channels) {
        auto channel = ch;  // Make a copy
        channel.selected = true;
        all_channels.add_channel_id(channel.id);
    }

    // Save to database
    if (service.create_channel_group(all_channels)) {
        printf("Group %s has %zu channels\n", all_channels.id.c_str(), all_channels.get_channel_count());
    } else {
        std::fprintf(stderr, "[AppState] ✗ Failed to create 'All Channels' group\n");
    }
}
