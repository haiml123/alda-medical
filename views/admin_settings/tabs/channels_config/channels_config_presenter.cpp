#include "channels_config_presenter.h"
#include <iostream>

namespace elda::views::channels_config {

    ChannelsConfigPresenter::ChannelsConfigPresenter(ChannelsConfigModel& model,
                                                       ChannelsConfigView& view)
        : model_(model)
        , view_(view)
    {
        setup_callbacks();
    }

    void ChannelsConfigPresenter::setup_callbacks() {
        callbacks_.on_channel_changed = [this](int id, const ChannelConfig& updated) {
            if (auto* ch = model_.get_channel(id)) {
                *ch = updated;
                std::cout << "[ChannelsConfig] Channel " << id << " updated\n";
            }
        };
    }

    void ChannelsConfigPresenter::render() {
        view_.render(model_.channels(), callbacks_);
    }

} // namespace elda::views::channels_config