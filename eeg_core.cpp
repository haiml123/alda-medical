#include "eeg_core.h"
#include "./views/channels_selector_modal/channels_group_presenter.h"

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