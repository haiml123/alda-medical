#include "monitoring_presenter.h"
#include "monitoring_model.h"
#include "monitoring_view.h"
#include "services/channel_management_service.h"
#include <iostream>

namespace elda {

MonitoringPresenter::MonitoringPresenter(
    MonitoringModel& model,
    MonitoringView& view,
    elda::channels_group::ChannelsGroupPresenter& channelsPresenter)
    : model_(model)
    , view_(view)
    , channelsPresenter_(channelsPresenter) {
}

void MonitoringPresenter::onEnter() {
    std::cout << "[Presenter] Enter monitoring" << std::endl;
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
    // ===== COLLECT DATA FROM MODEL =====
    MonitoringViewData viewData;
    viewData.chartData = &model_.getChartData();
    viewData.monitoring = model_.isMonitoring();
    viewData.canRecord = model_.canRecord();
    viewData.recordingActive = model_.isRecordingActive();
    viewData.currentlyPaused = model_.isCurrentlyPaused();
    viewData.windowSeconds = model_.getWindowSeconds();
    viewData.amplitudeMicroVolts = model_.getAmplitudeMicroVolts();
    viewData.sampleRateHz = model_.getSampleRateHz();
    viewData.groups = &model_.getAvailableGroups();
    viewData.activeGroupIndex = model_.getActiveGroupIndex();

    // ===== SETUP CALLBACKS =====
    MonitoringViewCallbacks callbacks;

    // Toolbar callbacks
    callbacks.onToggleMonitoring = [this]() {
        model_.toggleMonitoring();
    };

    callbacks.onToggleRecording = [this]() {
        model_.toggleRecording();
    };

    callbacks.onIncreaseWindow = [this]() {
        model_.increaseWindow();
    };

    callbacks.onDecreaseWindow = [this]() {
        model_.decreaseWindow();
    };

    callbacks.onIncreaseAmplitude = [this]() {
        model_.increaseAmplitude();
    };

    callbacks.onDecreaseAmplitude = [this]() {
        model_.decreaseAmplitude();
    };

    // Channel group callbacks
    callbacks.onCreateChannelGroup = [this]() {
        // Open modal in create mode
        channelsPresenter_.OpenWithChannels(
            elda::services::ChannelManagementService::GetInstance().GetAllChannels(),
            [this](const elda::models::ChannelsGroup& newGroup) {
                model_.applyChannelConfiguration(newGroup);
            }
        );
    };

    callbacks.onEditChannelGroup = [this](const std::string& groupName) {
        // Open modal in edit mode
        channelsPresenter_.Open(
            groupName,
            [this](const elda::models::ChannelsGroup& editedGroup) {
                model_.applyChannelConfiguration(editedGroup);
            }
        );
    };

    // ===== RENDER VIEW =====
    view_.render(viewData, callbacks);

    // ===== RENDER MODAL =====
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 modalPos = ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f - 150,
                             viewport->Pos.y + 100);
    ImVec2 modalSize = ImVec2(300, 550);
    channelsPresenter_.Render(modalPos, modalSize);
}

} // namespace elda