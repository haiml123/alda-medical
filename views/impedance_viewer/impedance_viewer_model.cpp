#include "impedance_viewer_model.h"

#include "services/channel_management_service.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

namespace elda::views::impedance_viewer
{

ImpedanceViewerModel::ImpedanceViewerModel(const std::vector<elda::models::Channel>& available_channels,
                                           AppStateManager& state_manager)
    : available_channels_(available_channels), state_manager_(state_manager)
{
    initialize_from_channels();
}

void ImpedanceViewerModel::update()
{
    // Update logic if needed (e.g., real-time impedance readings)
}

void ImpedanceViewerModel::initialize_from_channels()
{
    electrode_positions_.clear();
    original_positions_.clear();

    if (available_channels_.empty())
    {
        std::cout << "[ImpedanceViewerModel] No channels available\n";
        return;
    }

    electrode_positions_.reserve(available_channels_.size());

    bool has_positions = false;
    for (const auto& ch : available_channels_)
    {
        if (ch.impedance_x != 0.0f || ch.impedance_y != 0.0f)
        {
            has_positions = true;
            break;
        }
    }

    if (has_positions)
    {
        std::cout << "[ImpedanceViewerModel] Initializing from channel impedance positions\n";
        for (const auto& ch : available_channels_)
        {
            ElectrodePosition pos;
            pos.x = ch.impedance_x;
            pos.y = ch.impedance_y;
            pos.channel_id = ch.id;
            electrode_positions_.push_back(pos);
            original_positions_[ch.id] = {pos.x, pos.y};
        }
    }
    else
    {
        std::cout << "[ImpedanceViewerModel] No saved positions, generating default layout for "
                  << available_channels_.size() << " channels\n";
        initialize_default_positions();
    }

    std::cout << "[ImpedanceViewerModel] Initialized with " << electrode_positions_.size() << " electrode positions\n";
}

void ImpedanceViewerModel::initialize_default_positions()
{
    electrode_positions_.clear();
    original_positions_.clear();

    const float cx = 0.5f, cy = 0.5f;
    const size_t numChannels = available_channels_.size();
    size_t channel_idx = 0;

    const size_t innerCount = std::min(size_t(4), numChannels - channel_idx);
    for (size_t i = 0; i < innerCount && channel_idx < numChannels; ++i, ++channel_idx)
    {
        float ang = (i * 2.0f * float(M_PI) / 4.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.15f * std::cos(ang);
        p.y = cy + 0.15f * std::sin(ang);
        p.channel_id = available_channels_[channel_idx].id;
        electrode_positions_.push_back(p);
        original_positions_[p.channel_id] = {p.x, p.y};
    }

    const size_t middleCount = std::min(size_t(8), numChannels - channel_idx);
    for (size_t i = 0; i < middleCount && channel_idx < numChannels; ++i, ++channel_idx)
    {
        float ang = (i * 2.0f * float(M_PI) / 8.0f) - float(M_PI) * 0.5f;
        ElectrodePosition p;
        p.x = cx + 0.28f * std::cos(ang);
        p.y = cy + 0.28f * std::sin(ang);
        p.channel_id = available_channels_[channel_idx].id;
        electrode_positions_.push_back(p);
        original_positions_[p.channel_id] = {p.x, p.y};
    }

    while (channel_idx < numChannels)
    {
        const size_t remaining = numChannels - channel_idx;
        const size_t ringCount = std::min(size_t(8), remaining);
        const float ringRadius = 0.35f;

        for (size_t i = 0; i < ringCount; ++i, ++channel_idx)
        {
            float ang = (i * 2.0f * float(M_PI) / ringCount) - float(M_PI) * 0.5f;
            ElectrodePosition p;
            p.x = cx + ringRadius * std::cos(ang);
            p.y = cy + ringRadius * std::sin(ang);
            p.channel_id = available_channels_[channel_idx].id;
            electrode_positions_.push_back(p);
            original_positions_[p.channel_id] = {p.x, p.y};
        }
    }
}

const elda::models::Channel* ImpedanceViewerModel::get_channel_by_id(const std::string& id) const
{
    auto it = std::find_if(available_channels_.begin(),
                           available_channels_.end(),
                           [&id](const elda::models::Channel& ch)
                           {
                               return ch.id == id;
                           });
    if (it == available_channels_.end())
    {
        return nullptr;
    }
    return &(*it);
}

void ImpedanceViewerModel::update_electrode_position(size_t index, float x, float y)
{
    if (index >= electrode_positions_.size())
        return;
    if (!is_position_valid(x, y))
        return;

    electrode_positions_[index].x = x;
    electrode_positions_[index].y = y;

    std::cout << "[ImpedanceViewerModel] Updated electrode " << index << " to (" << x << ", " << y << ")\n";

    notify_position_changed();
}

void ImpedanceViewerModel::start_dragging(size_t index)
{
    if (index >= electrode_positions_.size())
        return;
    electrode_positions_[index].is_dragging = true;
    selected_electrode_index_ = static_cast<int>(index);
}

void ImpedanceViewerModel::stop_dragging(size_t index)
{
    if (index >= electrode_positions_.size())
        return;
    electrode_positions_[index].is_dragging = false;
}

void ImpedanceViewerModel::select_electrode(int index)
{
    selected_electrode_index_ = index;
}

void ImpedanceViewerModel::clear_selection()
{
    selected_electrode_index_ = -1;
}

bool ImpedanceViewerModel::is_position_valid(float x, float y) const
{
    const float dx = x - 0.5f;
    const float dy = y - 0.5f;
    return (dx * dx + dy * dy) <= cap_radius_ * cap_radius_;
}

void ImpedanceViewerModel::save_positions_to_state()
{
    std::cout << "[ImpedanceViewerModel] Saving positions to state\n";

    auto& channel_service = services::ChannelManagementService::get_instance();

    for (const auto& pos : electrode_positions_)
    {
        if (pos.channel_id.empty())
            continue;

        auto channelOpt = channel_service.get_channel(pos.channel_id);
        if (!channelOpt.has_value())
        {
            std::cout << "[ImpedanceViewerModel] Warning: Channel " << pos.channel_id
                      << " not found in state when saving impedance positions\n";
            continue;
        }

        elda::models::Channel updated = channelOpt.value();
        updated.impedance_x = pos.x;
        updated.impedance_y = pos.y;

        channel_service.update_channel(updated);
        std::cout << "[ImpedanceViewerModel] Saved impedance pos for channel " << pos.channel_id << "\n";
    }

    original_positions_.clear();
    for (const auto& pos : electrode_positions_)
    {
        original_positions_[pos.channel_id] = {pos.x, pos.y};
    }

    std::cout << "[ImpedanceViewerModel] Save complete\n";
}

void ImpedanceViewerModel::discard_changes()
{
    std::cout << "[ImpedanceViewerModel] Discarding changes, restoring original positions\n";

    for (auto& pos : electrode_positions_)
    {
        auto it = original_positions_.find(pos.channel_id);
        if (it != original_positions_.end())
        {
            pos.x = it->second.first;
            pos.y = it->second.second;
        }
    }

    clear_selection();
    notify_position_changed();

    std::cout << "[ImpedanceViewerModel] Discard complete\n";
}

void ImpedanceViewerModel::notify_position_changed()
{
    // Notify state manager if needed
}

}  // namespace elda::views::impedance_viewer
