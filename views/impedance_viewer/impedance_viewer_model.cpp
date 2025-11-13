#include "impedance_viewer_model.h"
#include "services/channel_management_service.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

namespace elda::impedance_viewer {

ImpedanceViewerModel::ImpedanceViewerModel(
    const std::vector<elda::models::Channel>& availableChannels,
    AppStateManager& stateManager)
    : availableChannels_(availableChannels)
    , stateManager_(stateManager) {
    InitializeFromChannels();
}

void ImpedanceViewerModel::InitializeFromChannels() {
    electrodePositions_.clear();
    originalPositions_.clear();

    if (availableChannels_.empty()) {
        std::cout << "[ImpedanceViewerModel] No channels available\n";
        return;
    }

    electrodePositions_.reserve(availableChannels_.size());

    // Check if any channels have positions set (non-zero)
    bool hasPositions = false;
    for (const auto& ch : availableChannels_) {
        if (ch.impedanceX != 0.0f || ch.impedanceY != 0.0f) {
            hasPositions = true;
            break;
        }
    }

    if (hasPositions) {
        // ✅ Load existing positions from channels
        std::cout << "[ImpedanceViewerModel] Loading positions from "
                  << availableChannels_.size() << " channels\n";

        for (const auto& ch : availableChannels_) {
            ElectrodePosition pos;
            pos.channelId = ch.id;

            // Convert from normalized [-1,1] to [0,1] display space
            // Stored: -1 to 1, Display: 0 to 1 with center at 0.5
            pos.x = (ch.impedanceX + 1.0f) * 0.5f;
            pos.y = (ch.impedanceY + 1.0f) * 0.5f;

            electrodePositions_.push_back(pos);

            // Save original positions for discard
            originalPositions_[ch.id] = {pos.x, pos.y};
        }
    } else {
        // ✅ No positions set - initialize with default concentric circles
        std::cout << "[ImpedanceViewerModel] No positions found, using defaults for "
                  << availableChannels_.size() << " channels\n";
        InitializeDefaultPositions();
    }

    NotifyPositionChanged();
}

void ImpedanceViewerModel::InitializeDefaultPositions() {
    // Create default concentric circle layout for available channels
    electrodePositions_.clear();
    originalPositions_.clear();

    const float cx = 0.5f, cy = 0.5f;
    const size_t numChannels = availableChannels_.size();
    size_t channelIdx = 0;

    // Inner ring (4 electrodes)
    const size_t innerCount = std::min(size_t(4), numChannels - channelIdx);
    for (size_t i = 0; i < innerCount && channelIdx < numChannels; ++i, ++channelIdx) {
        float ang = (i * 2.0f * float(M_PI) / 4.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.15f * std::cos(ang);
        p.y = cy + 0.15f * std::sin(ang);
        p.channelId = availableChannels_[channelIdx].id;
        electrodePositions_.push_back(p);
        originalPositions_[p.channelId] = {p.x, p.y};
    }

    // Middle ring (8 electrodes)
    const size_t middleCount = std::min(size_t(8), numChannels - channelIdx);
    for (size_t i = 0; i < middleCount && channelIdx < numChannels; ++i, ++channelIdx) {
        float ang = (i * 2.0f * float(M_PI) / 8.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.25f * std::cos(ang);
        p.y = cy + 0.25f * std::sin(ang);
        p.channelId = availableChannels_[channelIdx].id;
        electrodePositions_.push_back(p);
        originalPositions_[p.channelId] = {p.x, p.y};
    }

    // Outer ring (remaining electrodes in groups of 8)
    while (channelIdx < numChannels) {
        const size_t remaining = numChannels - channelIdx;
        const size_t ringCount = std::min(size_t(8), remaining);
        const float ringRadius = 0.35f;

        for (size_t i = 0; i < ringCount; ++i, ++channelIdx) {
            float ang = (i * 2.0f * float(M_PI) / ringCount) - float(M_PI) * 0.5f;
            ElectrodePosition p;
            p.x = cx + ringRadius * std::cos(ang);
            p.y = cy + ringRadius * std::sin(ang);
            p.channelId = availableChannels_[channelIdx].id;
            electrodePositions_.push_back(p);
            originalPositions_[p.channelId] = {p.x, p.y};
        }
    }

    std::cout << "[ImpedanceViewerModel] Initialized " << electrodePositions_.size()
              << " default positions\n";
}

const elda::models::Channel* ImpedanceViewerModel::GetChannelById(const std::string& id) const {
    auto it = std::find_if(availableChannels_.begin(), availableChannels_.end(),
        [&](const elda::models::Channel& ch){ return ch.id == id; });
    return (it != availableChannels_.end()) ? &(*it) : nullptr;
}

void ImpedanceViewerModel::UpdateElectrodePosition(size_t index, float x, float y) {
    if (index >= electrodePositions_.size()) return;
    if (!IsPositionValid(x, y)) return;

    // ✅ Update temporary position (not saved to state yet)
    electrodePositions_[index].x = x;
    electrodePositions_[index].y = y;

    NotifyPositionChanged();
}

void ImpedanceViewerModel::StartDragging(size_t index) {
    if (index >= electrodePositions_.size()) return;
    for (auto& e : electrodePositions_) e.isDragging = false;
    electrodePositions_[index].isDragging = true;
}

void ImpedanceViewerModel::StopDragging(size_t index) {
    if (index >= electrodePositions_.size()) return;
    electrodePositions_[index].isDragging = false;
}

void ImpedanceViewerModel::SelectElectrode(int index) {
    if (index < -1 || index >= static_cast<int>(electrodePositions_.size())) return;
    selectedElectrodeIndex_ = index;
    NotifyPositionChanged();
}

void ImpedanceViewerModel::ClearSelection() {
    selectedElectrodeIndex_ = -1;
    for (auto& e : electrodePositions_) e.isDragging = false;
    NotifyPositionChanged();
}

bool ImpedanceViewerModel::IsPositionValid(float x, float y) const {
    const float cx = 0.5f, cy = 0.5f;
    float dx = x - cx, dy = y - cy;
    return dx*dx + dy*dy <= capRadius_ * capRadius_;
}

void ImpedanceViewerModel::SavePositionsToState() {
    std::cout << "[ImpedanceViewerModel] Saving " << electrodePositions_.size()
              << " electrode positions to state\n";

    auto& channelService = services::ChannelManagementService::GetInstance();

    // ✅ Update each channel's impedance position and persist
    for (const auto& pos : electrodePositions_) {
        if (pos.channelId.empty()) continue;

        auto channelOpt = channelService.GetChannel(pos.channelId);
        if (!channelOpt.has_value()) {
            std::cout << "[ImpedanceViewerModel] Warning: Channel " << pos.channelId
                      << " not found in service\n";
            continue;
        }

        elda::models::Channel channel = channelOpt.value();

        // Convert from display space [0,1] to normalized [-1,1]
        // Display: 0 to 1 with center at 0.5, Stored: -1 to 1
        float normalizedX = (pos.x - 0.5f) * 2.0f;
        float normalizedY = (pos.y - 0.5f) * 2.0f;

        channel.SetImpedancePosition(normalizedX, normalizedY);

        // Persist to storage
        if (!channelService.UpdateChannel(channel)) {
            std::cout << "[ImpedanceViewerModel] Error: Failed to update channel "
                      << pos.channelId << "\n";
        }
    }

    // Update original positions to current (for future discards)
    originalPositions_.clear();
    for (const auto& pos : electrodePositions_) {
        originalPositions_[pos.channelId] = {pos.x, pos.y};
    }

    // Notify state manager
    // stateManager_.NotifyStateChanged(StateField::Monitoring);

    std::cout << "[ImpedanceViewerModel] Save complete\n";
}

void ImpedanceViewerModel::DiscardChanges() {
    std::cout << "[ImpedanceViewerModel] Discarding changes, restoring original positions\n";

    // ✅ Restore original positions
    for (auto& pos : electrodePositions_) {
        auto it = originalPositions_.find(pos.channelId);
        if (it != originalPositions_.end()) {
            pos.x = it->second.first;
            pos.y = it->second.second;
        }
    }

    ClearSelection();
    NotifyPositionChanged();

    std::cout << "[ImpedanceViewerModel] Discard complete\n";
}

void ImpedanceViewerModel::NotifyPositionChanged() {
    // Could notify state manager here if needed for real-time updates
    // For now, we only notify on explicit save
}

} // namespace elda::impedance_viewer