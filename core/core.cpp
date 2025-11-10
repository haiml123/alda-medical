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
    // Color palette for channels (can be customized)
    const char* colors[] = {
        "#FF6B6B", "#4ECDC4", "#45B7D1", "#FFA07A", "#98D8C8",
        "#F7DC6F", "#BB8FCE", "#85C1E2", "#F8B739", "#52C9B1",
        "#E74C3C", "#3498DB", "#2ECC71", "#F39C12", "#9B59B6",
        "#1ABC9C", "#E67E22", "#16A085", "#D35400", "#8E44AD"
    };

    int colorCount = sizeof(colors) / sizeof(colors[0]);

    availableChannels.clear();
    availableChannels.reserve(CHANNELS);

    for (int i = 0; i < CHANNELS; ++i) {
        char id[16], name[16];
        std::snprintf(id, sizeof(id), "ch_%d", i);
        std::snprintf(name, sizeof(name), "Channel %02d", i + 1);

        // Cycle through colors
        const char* color = colors[i % colorCount];

        availableChannels.emplace_back(id, name, color);
    }
}

void AppState::InitializeGroupChannels() {
    auto& service = elda::services::ChannelManagementService::GetInstance();
    availableGroups = service.GetAllChannelGroups();
}

void AppState::CreateDefaultGroups() {
    auto& service = elda::services::ChannelManagementService::GetInstance();

    // =========================================================================
    // DEFAULT GROUP 1: All 64 Channels
    // =========================================================================
    if (!service.ChannelGroupExists("All Channels")) {
        elda::models::ChannelsGroup allChannels("All Channels");
        allChannels.description = "All 64 channels";

        // Add all 64 channels
        for (size_t i = 0; i < availableChannels.size(); ++i) {
            auto ch = availableChannels[i];
            ch.selected = true;
            allChannels.channels.push_back(ch);
        }

        if (service.CreateChannelGroup(allChannels)) {
            std::printf("[AppState] Created 'All Channels' group (%zu channels)\n",
                       allChannels.getSelectedCount());
        }
    }

    // =========================================================================
    // DEFAULT GROUP 2: Frontal (First 16 channels)
    // =========================================================================
    if (!service.ChannelGroupExists("Frontal")) {
        elda::models::ChannelsGroup frontal("Frontal");
        frontal.description = "Frontal region (channels 1-16)";

        // Add first 16 channels
        for (int i = 0; i < 16 && i < static_cast<int>(availableChannels.size()); ++i) {
            auto ch = availableChannels[i];
            ch.selected = true;
            frontal.channels.push_back(ch);
        }

        if (service.CreateChannelGroup(frontal)) {
            std::printf("[AppState] Created 'Frontal' group (%zu channels)\n",
                       frontal.getSelectedCount());
        }
    }

    // =========================================================================
    // DEFAULT GROUP 3: Posterior (Last 16 channels)
    // =========================================================================
    if (!service.ChannelGroupExists("Posterior")) {
        elda::models::ChannelsGroup posterior("Posterior");
        posterior.description = "Posterior region (channels 49-64)";

        // Add last 16 channels
        int startIdx = std::max(0, static_cast<int>(availableChannels.size()) - 16);
        for (size_t i = startIdx; i < availableChannels.size(); ++i) {
            auto ch = availableChannels[i];
            ch.selected = true;
            posterior.channels.push_back(ch);
        }

        if (service.CreateChannelGroup(posterior)) {
            std::printf("[AppState] Created 'Posterior' group (%zu channels)\n",
                       posterior.getSelectedCount());
        }
    }
}