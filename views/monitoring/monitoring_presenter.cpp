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
    , channelsPresenter_(channelsPresenter)
    , channelModalPosition_(0, 0)
    , useCustomModalPosition_(false) {
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
        // Reset to default positioning for create mode
        useCustomModalPosition_ = false;

        // Open modal in create mode
        channelsPresenter_.OpenWithChannels(
            elda::services::ChannelManagementService::GetInstance().GetAllChannels(),
            [this](const elda::models::ChannelsGroup& newGroup) {
                model_.applyChannelConfiguration(newGroup);
            }
        );
    };

    callbacks.onEditChannelGroup = [this](const std::string& groupName, const ui::TabBounds* bounds) {
        // Calculate modal position based on tab bounds
        if (bounds) {
            // Position modal centered below the tab
            const float modalWidth = 300.0f;
            const float modalSpacing = 10.0f;

            channelModalPosition_ = ImVec2(
                bounds->x + (bounds->width - modalWidth) * 0.5f,  // Center horizontally
                bounds->y + bounds->height + modalSpacing          // Below tab with spacing
            );
            useCustomModalPosition_ = true;

            std::cout << "[Presenter] Opening modal at tab position ("
                      << channelModalPosition_.x << ", " << channelModalPosition_.y << ")" << std::endl;
        } else {
            // Fallback to default positioning
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

    // ===== RENDER VIEW =====
    view_.render(viewData, callbacks);

    // ===== RENDER MODAL WITH SMART POSITIONING =====
    ImVec2 modalPos;
    ImVec2 modalSize = ImVec2(300, 550);

    if (useCustomModalPosition_) {
        // Use the position calculated from tab bounds
        modalPos = channelModalPosition_;

        // Ensure modal stays within viewport bounds
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Clamp X position
        if (modalPos.x < viewport->Pos.x) {
            modalPos.x = viewport->Pos.x + 10.0f;
        }
        if (modalPos.x + modalSize.x > viewport->Pos.x + viewport->Size.x) {
            modalPos.x = viewport->Pos.x + viewport->Size.x - modalSize.x - 10.0f;
        }

        // Clamp Y position
        if (modalPos.y < viewport->Pos.y) {
            modalPos.y = viewport->Pos.y + 10.0f;
        }
        if (modalPos.y + modalSize.y > viewport->Pos.y + viewport->Size.y) {
            // If modal would go below screen, position it above the tab instead
            modalPos.y = channelModalPosition_.y - modalSize.y - 10.0f;

            // Still too high? Center it on screen
            if (modalPos.y < viewport->Pos.y) {
                modalPos.y = viewport->Pos.y + (viewport->Size.y - modalSize.y) * 0.5f;
            }
        }
    } else {
        // Default positioning (center of screen)
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        modalPos = ImVec2(
            viewport->Pos.x + (viewport->Size.x - modalSize.x) * 0.5f,
            viewport->Pos.y + 100.0f
        );
    }

    modalSize.y = 0;
    modalSize.x = 0;
    channelsPresenter_.Render(modalPos, modalSize);
}

} // namespace elda