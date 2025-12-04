#pragma once

#include "channels_config_model.h"
#include "channels_config_view.h"

namespace elda::views::channels_config
{

class ChannelsConfigPresenter
{
  public:
    ChannelsConfigPresenter(ChannelsConfigModel& model, ChannelsConfigView& view);

    void render();

  private:
    void setup_callbacks();

    ChannelsConfigModel& model_;
    ChannelsConfigView& view_;
    ChannelsConfigCallbacks callbacks_;
};

}  // namespace elda::views::channels_config
