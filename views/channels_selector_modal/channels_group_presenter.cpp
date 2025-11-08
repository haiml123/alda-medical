#include "channels_group_presenter.h"

namespace elda::channels_group {

    ChannelsGroupPresenter::ChannelsGroupPresenter()
        : model_(std::make_unique<ChannelsGroupModel>())
        , view_(std::make_unique<ChannelsGroupView>())
        , onConfirmCallback_(nullptr) {

        SetupViewCallbacks();
    }

    // ========================================================================
    // PUBLIC API
    // ========================================================================

    void ChannelsGroupPresenter::Open(const std::string& groupName, OnConfirmCallback callback) {
        onConfirmCallback_ = callback;

        // Load group from service via model
        if (model_->LoadChannelGroup(groupName)) {
            view_->SetVisible(true);
            UpdateView();
        } else {
            // Group not found - could show error or create new
            model_->Clear();
            model_->SetGroupName(groupName);
            view_->SetVisible(true);
            UpdateView();
        }
    }

    void ChannelsGroupPresenter::OpenWithActiveGroup(OnConfirmCallback callback) {
        onConfirmCallback_ = callback;

        // Load active group from service via model
        if (model_->LoadActiveChannelGroup()) {
            view_->SetVisible(true);
            UpdateView();
        } else {
            // No active group - start with empty
            model_->Clear();
            view_->SetVisible(true);
            UpdateView();
        }
    }

    void ChannelsGroupPresenter::OpenWithChannels(
        const std::vector<models::Channel>& channels,
        OnConfirmCallback callback
    ) {
        onConfirmCallback_ = callback;
        model_->Clear();
        model_->SetChannels(channels);
        view_->SetVisible(true);
        UpdateView();
    }

    void ChannelsGroupPresenter::Close() {
        view_->SetVisible(false);
        model_->Clear();
        onConfirmCallback_ = nullptr;
    }

    void ChannelsGroupPresenter::Render(ImVec2 buttonPos, ImVec2 buttonSize) {
        if (!view_->IsVisible()) return;

        // Calculate position below button
        ImVec2 modalPos = ImVec2(buttonPos.x, buttonPos.y + buttonSize.y + 2.0f);
        view_->SetPosition(modalPos);

        // Get current state from model
        std::string errorMessage;
        bool canConfirm = model_->CanConfirm(errorMessage);

        // Render view with current model state
        view_->Render(
            model_->GetGroupName(),
            model_->GetChannels(),
            model_->GetSelectedCount(),
            model_->GetTotalCount(),
            canConfirm
        );
    }

    bool ChannelsGroupPresenter::IsOpen() const {
        return view_->IsVisible();
    }

    // ========================================================================
    // VIEW EVENT HANDLERS
    // ========================================================================

    void ChannelsGroupPresenter::OnGroupNameChanged(const std::string& newName) {
        // User typed in the name field - update model
        model_->SetGroupName(newName);
        // View will be updated on next render cycle
    }

    void ChannelsGroupPresenter::OnChannelSelectionChanged(size_t index, bool selected) {
        // User clicked a channel checkbox - update model
        model_->SelectChannel(index, selected);
        // View will be updated on next render cycle
    }

    void ChannelsGroupPresenter::OnSelectAllChannels(bool selected) {
        // User clicked Select All or Clear All - update model
        model_->SelectAllChannels(selected);
        // View will be updated on next render cycle
    }

    void ChannelsGroupPresenter::OnConfirm() {
        // User clicked Confirm button

        // Validate via model
        std::string errorMessage;
        if (!model_->CanConfirm(errorMessage)) {
            // Could show error message in UI
            // For now, just don't proceed
            return;
        }

        // Save to service via model
        if (model_->SaveChannelGroup()) {
            // Create group object for callback
            models::ChannelsGroup group(model_->GetGroupName());
            group.channels = model_->GetChannels();

            // Notify callback
            if (onConfirmCallback_) {
                onConfirmCallback_(group);
            }

            // Close the modal
            Close();
        } else {
            // Save failed - could show error
            // For now, just don't close
        }
    }

    void ChannelsGroupPresenter::OnCancel() {
        // User closed the window or cancelled
        Close();
    }

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================

    void ChannelsGroupPresenter::UpdateView() {
        // This is called after model changes to ensure view reflects current state
        // In ImGui's immediate mode, the view will query model state on next render
        // So this is mostly a placeholder for future enhancements
    }

    void ChannelsGroupPresenter::SetupViewCallbacks() {
        // Wire up view callbacks to presenter event handlers
        IChannelsGroupViewCallbacks callbacks;

        callbacks.onGroupNameChanged = [this](const std::string& name) {
            OnGroupNameChanged(name);
        };

        callbacks.onChannelSelectionChanged = [this](size_t index, bool selected) {
            OnChannelSelectionChanged(index, selected);
        };

        callbacks.onSelectAllChannels = [this](bool selected) {
            OnSelectAllChannels(selected);
        };

        callbacks.onConfirm = [this]() {
            OnConfirm();
        };

        callbacks.onCancel = [this]() {
            OnCancel();
        };

        view_->SetCallbacks(callbacks);
    }

} // namespace elda::channels_group