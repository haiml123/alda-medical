#include "monitoring_presenter.h"
#include "monitoring_model.h"
#include "monitoring_view.h"
#include <iostream>

namespace elda {

MonitoringPresenter::MonitoringPresenter(
    MonitoringModel& model,
    MonitoringView& view,
    elda::channels_group::ChannelsGroupPresenter& channelsPresenter)
    : model_(model)
    , view_(view)
    , channelsPresenter_(channelsPresenter)
    , channelModalPosition_(0, 0)
    , useCustomModalPosition_(false) {

    // Setup callbacks once during construction
    setupCallbacks();
}

void MonitoringPresenter::onEnter() {
    std::cout << "[Presenter] Enter monitoring" << std::endl;

    // Reset cache on screen enter
    cachedState_ = CachedViewState();

    model_.startAcquisition();
}

void MonitoringPresenter::onExit() {
    std::cout << "[Presenter] Exit monitoring" << std::endl;
    model_.stopAcquisition();
}

void MonitoringPresenter::update(float deltaTime) {
    model_.update(deltaTime);
}

void MonitoringPresenter::render() {
    // =========================================================================
    // STEP 1: COLLECT DATA FROM MODEL (optimized with caching)
    // =========================================================================
    MonitoringViewData viewData;

    // Always get the chart data pointer (this is fast)
    viewData.chartData = &model_.getChartData();

    // Get frequently-changing state (these need to be checked every frame)
    viewData.monitoring = model_.isMonitoring();
    viewData.canRecord = model_.canRecord();
    viewData.recordingActive = model_.isRecordingActive();
    viewData.currentlyPaused = model_.isCurrentlyPaused();

    // Use cached values for rarely-changing state
    if (cachedState_.needsRefresh()) {
        refreshCachedState();
    }

    viewData.windowSeconds = cachedState_.windowSeconds;
    viewData.amplitudeMicroVolts = cachedState_.amplitudeMicroVolts;
    viewData.sampleRateHz = cachedState_.sampleRateHz;
    viewData.activeGroupIndex = cachedState_.activeGroupIndex;

    // Groups pointer (fast, just a pointer)
    viewData.groups = &model_.getAvailableGroups();

    // Increment frame counter
    cachedState_.framesSinceUpdate++;

    // =========================================================================
    // STEP 2: UPDATE DYNAMIC CALLBACKS (only those that capture local state)
    // =========================================================================
    // Most callbacks are already set up in constructor.
    // Only these two need to be updated because they capture view-specific state:

    callbacks_.onCreateChannelGroup = [this]() {
        std::cout << "[Presenter] Create new channel group" << std::endl;
        channelsPresenter_.OpenWithActiveGroup(
            [this](const elda::models::ChannelsGroup& newGroup) {
                model_.applyChannelConfiguration(newGroup);
            }
        );
    };

    callbacks_.onEditChannelGroup = [this](const std::string& groupName, const ui::TabBounds* bounds) {
        std::cout << "[Presenter] Edit channel group: " << groupName << std::endl;

        // Calculate modal position based on tab bounds
        if (bounds) {
            useCustomModalPosition_ = true;
            channelModalPosition_.x = bounds->x;
            channelModalPosition_.y = bounds->y + bounds->height;
            std::cout << "[Presenter] Using tab position: ("
                      << channelModalPosition_.x << ", "
                      << channelModalPosition_.y << ")" << std::endl;
        } else {
            useCustomModalPosition_ = false;
            std::cout << "[Presenter] Warning: No bounds provided, using default position" << std::endl;
        }

        // Open modal in edit mode
        channelsPresenter_.Open(
            groupName,
            [this](const elda::models::ChannelsGroup& editedGroup) {
                model_.applyChannelConfiguration(editedGroup);
            }
        );
    };

    // =========================================================================
    // STEP 3: RENDER VIEW
    // =========================================================================
    view_.render(viewData, callbacks_);

    // =========================================================================
    // STEP 4: RENDER MODAL (only if open - optimization!)
    // =========================================================================
    if (channelsPresenter_.IsOpen()) {
        ImVec2 modalPos;
        ImVec2 modalSize = ImVec2(300, 550);

        if (useCustomModalPosition_) {
            modalPos = channelModalPosition_;
        } else {
            modalPos = calculateDefaultModalPosition(modalSize);
        }

        channelsPresenter_.Render(modalPos);
    }
}

// =============================================================================
// PRIVATE HELPER METHODS
// =============================================================================

void MonitoringPresenter::refreshCachedState() {
    cachedState_.windowSeconds = model_.getWindowSeconds();
    cachedState_.amplitudeMicroVolts = model_.getAmplitudeMicroVolts();
    cachedState_.sampleRateHz = model_.getSampleRateHz();
    cachedState_.activeGroupIndex = model_.getActiveGroupIndex();
    cachedState_.framesSinceUpdate = 0;
}


ImVec2 MonitoringPresenter::calculateDefaultModalPosition(ImVec2 modalSize) const {
    // Default positioning (near top-center of screen)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    return ImVec2(
        viewport->Pos.x + (viewport->Size.x - modalSize.x) * 0.5f,
        viewport->Pos.y + 100.0f
    );
}

void MonitoringPresenter::setupCallbacks() {
    // =========================================================================
    // TOOLBAR CALLBACKS (static, set once)
    // =========================================================================
    callbacks_.onToggleMonitoring = [this]() {
        model_.toggleMonitoring();
    };

    callbacks_.onToggleRecording = [this]() {
        model_.toggleRecording();
    };

    callbacks_.onIncreaseWindow = [this]() {
        model_.increaseWindow();
        // Invalidate cache since window size changed
        cachedState_.windowSeconds = -1;
    };

    callbacks_.onDecreaseWindow = [this]() {
        model_.decreaseWindow();
        // Invalidate cache since window size changed
        cachedState_.windowSeconds = -1;
    };

    callbacks_.onIncreaseAmplitude = [this]() {
        model_.increaseAmplitude();
        // Invalidate cache since amplitude changed
        cachedState_.amplitudeMicroVolts = -1;
    };

    callbacks_.onDecreaseAmplitude = [this]() {
        model_.decreaseAmplitude();
        // Invalidate cache since amplitude changed
        cachedState_.amplitudeMicroVolts = -1;
    };

    // Note: onCreateChannelGroup and onEditChannelGroup are set in render()
    // because they need to capture view-specific state (modal positioning)
}

} // namespace elda