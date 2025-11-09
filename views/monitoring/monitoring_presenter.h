#ifndef ELDA_MONITORING_PRESENTER_H
#define ELDA_MONITORING_PRESENTER_H

#include "views/channels_selector_modal/channels_group_presenter.h"

namespace elda {
    class MonitoringModel;
    class MonitoringView;
}

namespace elda {

    class MonitoringPresenter {
    public:
        MonitoringPresenter(
            MonitoringModel& model,
            MonitoringView& view,
            elda::channels_group::ChannelsGroupPresenter& channelsPresenter);

        void onEnter();
        void onExit();
        void update(float deltaTime);
        void render();

    private:
        MonitoringModel& model_;
        MonitoringView& view_;
        elda::channels_group::ChannelsGroupPresenter& channelsPresenter_;
    };

} // namespace elda

#endif // ELDA_MONITORING_PRESENTER_H