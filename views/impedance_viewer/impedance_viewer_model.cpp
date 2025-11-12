#include "impedance_viewer_model.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace elda::impedance_viewer {

ImpedanceViewerModel::ImpedanceViewerModel(
    const std::vector<elda::models::Channel>& availableChannels,
    AppStateManager& stateManager)
    : availableChannels_(availableChannels)
    , stateManager_(stateManager) {
    InitializeDefaultPositions();
}

void ImpedanceViewerModel::InitializeDefaultPositions() {
    electrodePositions_.clear();
    electrodePositions_.reserve(20);

    const float cx = 0.5f, cy = 0.5f;

    // 4 inner
    for (int i = 0; i < 4; ++i) {
        float ang = (i * 2.0f * float(M_PI) / 4.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.15f * std::cos(ang);
        p.y = cy + 0.15f * std::sin(ang);
        electrodePositions_.push_back(p);
    }
    // 8 middle
    for (int i = 0; i < 8; ++i) {
        float ang = (i * 2.0f * float(M_PI) / 8.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.25f * std::cos(ang);
        p.y = cy + 0.25f * std::sin(ang);
        electrodePositions_.push_back(p);
    }
    // 8 outer
    for (int i = 0; i < 8; ++i) {
        float ang = (i * 2.0f * float(M_PI) / 8.0f) - float(M_PI) * 0.5f + float(M_PI)/16.0f;
        ElectrodePosition p;
        p.x = cx + 0.35f * std::cos(ang);
        p.y = cy + 0.35f * std::sin(ang);
        electrodePositions_.push_back(p);
    }

    // Auto-assign first 20 names if available
    for (size_t i = 0; i < electrodePositions_.size() && i < availableChannels_.size(); ++i) {
        electrodePositions_[i].channelId = availableChannels_[i].id;
    }

    NotifyPositionChanged();
}

const elda::models::Channel* ImpedanceViewerModel::GetChannelById(const std::string& id) const {
    auto it = std::find_if(availableChannels_.begin(), availableChannels_.end(),
        [&](const elda::models::Channel& ch){ return ch.id == id; });
    return (it != availableChannels_.end()) ? &(*it) : nullptr;
}

void ImpedanceViewerModel::UpdateElectrodePosition(size_t index, float x, float y) {
    if (index >= electrodePositions_.size()) return;
    if (!IsPositionValid(x, y)) return;
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

void ImpedanceViewerModel::NotifyPositionChanged() {
    // stateManager_.NotifyStateChanged(StateField::Monitoring);
}

} // namespace elda::impedance_viewer
