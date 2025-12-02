#pragma once
#include "views/channels_selector_modal/channels_group_presenter.h"
#include "imgui.h"
#include "monitoring_view.h"
#include "monitoring_model.h"

namespace elda::views::monitoring {

    class MonitoringPresenter {
    public:
        MonitoringPresenter(
            MonitoringModel& model,
            MonitoringView& view,
            channels_selector::ChannelsGroupPresenter& channels_presenter);

        void on_enter();
        void on_exit();
        void update(float delta_time);
        void render();

    private:
        MonitoringModel& model_;
        MonitoringView& view_;
        std::unique_ptr<channels_selector::ChannelsGroupPresenter> channels_presenter_;
        // ========================================================================
        // CACHED STATE - Avoid redundant queries
        // ========================================================================
        struct CachedViewState {
            // Values that rarely change - refresh periodically
            int window_seconds = -1;
            int amplitude_micro_volts = -1;
            double sample_rate_hz = -1;
            int active_group_index = -1;

            // Frame counter for periodic refresh
            int frames_since_update = 0;

            // Refresh interval (frames)
            static constexpr int REFRESH_INTERVAL = 10;

            bool needs_refresh() const {
                return frames_since_update >= REFRESH_INTERVAL || window_seconds == -1;
            }
        };
        CachedViewState cached_state_;

        // ========================================================================
        // PRE-BUILT CALLBACKS - Avoid lambda allocation every frame
        // ========================================================================
        MonitoringViewCallbacks callbacks_;

        // ========================================================================
        // MODAL POSITIONING
        // ========================================================================
        ImVec2 channel_modal_position_;
        bool use_custom_modal_position_ = false;

        // ========================================================================
        // HELPER METHODS
        // ========================================================================

        /**
         * Refresh cached state from model
         */
        void refresh_cached_state();

        /**
         * Calculate default centered modal position
         */
        ImVec2 calculate_default_modal_position(ImVec2 modal_size) const;

        /**
         * Setup all callbacks (called once in constructor)
         */
        void setup_callbacks();
    };

}