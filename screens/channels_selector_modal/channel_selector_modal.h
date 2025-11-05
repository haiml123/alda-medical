#pragma once
#include "imgui.h"
#include <string>
#include <vector>

// Channel data structure
struct Channel {
    std::string id;
    std::string name;
    std::string color;  // hex color like "#FF0000" or RGB format
    bool selected;

    Channel(const std::string& id_, const std::string& name_, const std::string& color_)
        : id(id_), name(name_), color(color_), selected(false) {}
};

// Modal state and configuration
struct ChannelSelectorModal {
    bool isOpen = false;
    std::string inputName;
    std::vector<Channel>* channelsRef = nullptr;  // Reference to channels in AppState

    // Callback when user confirms selection
    using OnConfirmCallback = void(*)(const std::string& name, const std::vector<Channel>& channels);
    OnConfirmCallback onConfirm = nullptr;

    // Open the modal
    void Open(std::vector<Channel>* channels, OnConfirmCallback callback = nullptr);

    // Draw the modal (call this every frame)
    void Draw(ImVec2 buttonPos, ImVec2 buttonSize);

    // Close the modal
    void Close();

private:
    void DrawChannelsList();
};