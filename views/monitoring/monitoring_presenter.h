#ifndef ELDA_MONITORING_PRESENTER_H
#define ELDA_MONITORING_PRESENTER_H

#include "monitoring_model.h"
#include "monitoring_view.h"

namespace elda {

    class MonitoringPresenter {
    public:
        MonitoringPresenter(MonitoringModel& model, MonitoringView& view);
        ~MonitoringPresenter() = default;

        void onEnter();
        void onExit();
        void update(float deltaTime);
        void render();

    private:
        MonitoringModel& model_;
        MonitoringView& view_;
    };

} // namespace elda

#endif