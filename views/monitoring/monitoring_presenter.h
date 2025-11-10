#ifndef ELDA_MONITORING_PRESENTER_H
#define ELDA_MONITORING_PRESENTER_H

#include "views/channels_selector_modal/channels_group_presenter.h"
#include "imgui.h"
#include "monitoring_view.h"

namespace elda {
    class MonitoringModel;
    class MonitoringView;
    struct MonitoringViewCallbacks;
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

        // ========================================================================
        // CACHED STATE - Avoid redundant queries
        // ========================================================================
        struct CachedViewState {
            // Values that rarely change - refresh periodically
            int windowSeconds = -1;
            int amplitudeMicroVolts = -1;
            double sampleRateHz = -1;
            int activeGroupIndex = -1;

            // Frame counter for periodic refresh
            int framesSinceUpdate = 0;

            // Refresh interval (frames)
            static constexpr int REFRESH_INTERVAL = 10;

            bool needsRefresh() const {
                return framesSinceUpdate >= REFRESH_INTERVAL || windowSeconds == -1;
            }
        };
        CachedViewState cachedState_;

        // ========================================================================
        // PRE-BUILT CALLBACKS - Avoid lambda allocation every frame
        // ========================================================================
        MonitoringViewCallbacks callbacks_;

        // ========================================================================
        // MODAL POSITIONING
        // ========================================================================
        ImVec2 channelModalPosition_;
        bool useCustomModalPosition_ = false;

        // ========================================================================
        // HELPER METHODS
        // ========================================================================

        /**
         * Refresh cached state from model
         */
        void refreshCachedState();

        /**
         * Calculate default centered modal position
         */
        ImVec2 calculateDefaultModalPosition(ImVec2 modalSize) const;

        /**
         * Setup all callbacks (called once in constructor)
         */
        void setupCallbacks();
    };

} // namespace elda

#endif // ELDA_MONITORING_PRESENTER_H