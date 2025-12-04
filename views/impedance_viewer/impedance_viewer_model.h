#pragma once
#include "core/app_state_manager.h"
#include "models/channel.h"

#include <map>
#include <string>
#include <vector>

namespace elda::views::impedance_viewer
{

struct ElectrodePosition
{
    float x = 0.5f;
    float y = 0.5f;
    std::string channel_id;
    bool is_dragging = false;
};

class ImpedanceViewerModel
{
  public:
    ImpedanceViewerModel(const std::vector<elda::models::Channel>& available_channels, AppStateManager& state_manager);

    const std::vector<ElectrodePosition>& get_electrode_positions() const
    {
        return electrode_positions_;
    }
    const std::vector<elda::models::Channel>& get_available_channels() const
    {
        return available_channels_;
    }
    int get_selected_electrode_index() const
    {
        return selected_electrode_index_;
    }

    void update();
    void initialize_from_channels();
    void update_electrode_position(size_t index, float x, float y);
    void start_dragging(size_t index);
    void stop_dragging(size_t index);
    void select_electrode(int index);
    void clear_selection();

    void save_positions_to_state();
    void discard_changes();

    const models::Channel* get_channel_by_id(const std::string& id) const;
    bool is_position_valid(float x, float y) const;

  private:
    void notify_position_changed();
    void initialize_default_positions();

    std::vector<ElectrodePosition> electrode_positions_;
    std::map<std::string, std::pair<float, float>> original_positions_;
    std::vector<models::Channel> available_channels_;
    AppStateManager& state_manager_;

    int selected_electrode_index_ = -1;
    const float cap_radius_ = 0.48f;
};

}  // namespace elda::views::impedance_viewer
