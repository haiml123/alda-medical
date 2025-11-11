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

    // Set handlers once
    setupCallbacks();
}

void MonitoringPresenter::onEnter() {
    std::cout << "[Presenter] Enter monitoring" << std::endl;

    model_.StartAcquisition();

    // if you still want notifications, keep the observer — but no cache/dirty flags
    model_.addStateObserver([this](StateField field) {
        std::cout << "[Presenter] State changed: " << static_cast<int>(field) << std::endl;
    });
}

void MonitoringPresenter::onExit() {
    std::cout << "[Presenter] Exit monitoring" << std::endl;
    model_.StopAcquisition();
}

void MonitoringPresenter::update(float deltaTime) {
    model_.Update(deltaTime);
}

void MonitoringPresenter::render() {
    // =========================================================================
    // STEP 1: COLLECT DATA FRESH EVERY FRAME (no caching)
    // =========================================================================
    MonitoringViewData viewData;

    // heavy data as pointer/refs (no copies)
    viewData.chartData          = &model_.GetChartData();
    viewData.groups             = &model_.GetAvailableGroups();

    // simple scalars/booleans — fetch every frame (cost is trivial)
    viewData.monitoring         = model_.IsMonitoring();
    viewData.canRecord          = model_.CanRecord();
    viewData.recordingActive    = model_.IsRecordingActive();
    viewData.currentlyPaused    = model_.IsCurrentlyPaused();
    viewData.windowSeconds      = model_.GetWindowSeconds();
    viewData.amplitudeMicroVolts= model_.GetAmplitudeMicroVolts();
    viewData.sampleRateHz       = model_.GetSampleRateHz();
    viewData.activeGroupIndex   = model_.GetActiveGroupIndex();
    viewData.selectedChannels   = &model_.GetSelectedChannels();

    // =========================================================================
    // STEP 2: RENDER VIEW
    // =========================================================================
    view_.render(viewData, callbacks_);

    // =========================================================================
    // STEP 3: RENDER MODAL IF OPEN
    // =========================================================================
    if (channelsPresenter_.IsOpen()) {
        ImVec2 modalSize = ImVec2(300, 550);
        ImVec2 modalPos = useCustomModalPosition_
            ? channelModalPosition_
            : calculateDefaultModalPosition(modalSize);

        channelsPresenter_.Render(modalPos);
    }
}

ImVec2 MonitoringPresenter::calculateDefaultModalPosition(ImVec2 modalSize) const {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    return ImVec2(
        viewport->Pos.x + (viewport->Size.x - modalSize.x) * 0.5f,
        viewport->Pos.y + 100.0f
    );
}

void MonitoringPresenter::setupCallbacks() {
    // toolbar
    callbacks_.onToggleMonitoring   = [this]() { model_.ToggleMonitoring();   };
    callbacks_.onToggleRecording    = [this]() { model_.ToggleRecording();    };
    callbacks_.onIncreaseWindow     = [this]() { model_.IncreaseWindow();     };
    callbacks_.onDecreaseWindow     = [this]() { model_.DecreaseWindow();     };
    callbacks_.onIncreaseAmplitude  = [this]() { model_.IncreaseAmplitude();  };
    callbacks_.onDecreaseAmplitude  = [this]() { model_.DecreaseAmplitude();  };

    callbacks_.onCreateChannelGroup = [this]() {
        channelsPresenter_.Open("",
            [this](const models::ChannelsGroup& newGroup) {
                model_.ApplyChannelConfiguration(newGroup);
                model_.OnGroupSelected(newGroup);
            }
        );
    };

    callbacks_.onGroupSelected = [this](const models::ChannelsGroup* group) {
        if (!group) return;
        model_.OnGroupSelected(*group);
    };

    callbacks_.onEditChannelGroup = [this](const std::string& id, const ui::TabBounds* bounds) {
        if (bounds) {
            useCustomModalPosition_ = true;
            channelModalPosition_.x = bounds->x;
            channelModalPosition_.y = bounds->y + bounds->height;
        } else {
            useCustomModalPosition_ = false;
        }

        channelsPresenter_.Open(
            id,
            [this](const elda::models::ChannelsGroup& editedGroup) {
                model_.ApplyChannelConfiguration(editedGroup);
                model_.OnGroupSelected(editedGroup);
            }
        );
    };
}

} // namespace elda
