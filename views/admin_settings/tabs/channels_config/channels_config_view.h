#pragma once

#include "channels_config_model.h"
#include "imgui.h"

#include <functional>
#include <vector>

namespace elda::views::channels_config
{

struct ChannelsConfigCallbacks
{
    std::function<void(int, const ChannelConfig&)> on_channel_changed;  // id, new config
};

class ChannelsConfigView
{
  public:
    ChannelsConfigView() = default;

    void render(const std::vector<ChannelConfig>& channels, const ChannelsConfigCallbacks& callbacks);

  private:
    void render_table(const std::vector<ChannelConfig>& channels, const ChannelsConfigCallbacks& callbacks);
    void render_channel_row(const ChannelConfig& channel, const ChannelsConfigCallbacks& callbacks);
};

}  // namespace elda::views::channels_config