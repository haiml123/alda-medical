#include "channels_group_presenter.h"

namespace elda::views::channels_selector
{

ChannelsGroupPresenter::ChannelsGroupPresenter(elda::AppStateManager& state_manager)
    : model_(std::make_unique<ChannelsGroupModel>(state_manager)),
      view_(std::make_unique<ChannelsGroupView>()),
      on_confirm_callback_(nullptr),
      on_delete_callback_(nullptr)
{
    setup_view_callbacks();
}

// ========================================================================
// PUBLIC API
// ========================================================================

void ChannelsGroupPresenter::open(const std::string& group_id,
                                  OnConfirmCallback callback,
                                  OnDeleteCallback delete_callback)
{
    on_confirm_callback_ = callback;
    on_delete_callback_ = delete_callback;

    model_->clear();  // Start fresh

    if (!group_id.empty())
    {
        // Edit existing group
        if (!model_->load_channel_group_by_id(group_id))
        {
            std::fprintf(stderr, "[ChannelsGroupPresenter] Failed to load group: %s\n", group_id.c_str());
            model_->clear();
        }
    }
    else
    {
        // New group - just load all channels with none selected
        model_->set_channels(model_->get_all_channels());
    }

    view_->set_visible(true);
    update_view();
}

void ChannelsGroupPresenter::set_on_groups_changed_callback(OnGroupsChangedCallback callback)
{
    // Wire this callback through to the model
    model_->set_on_groups_changed_callback(callback);
}

void ChannelsGroupPresenter::close()
{
    view_->set_visible(false);
    model_->clear();
    on_confirm_callback_ = nullptr;
    on_delete_callback_ = nullptr;
}

void ChannelsGroupPresenter::render(ImVec2 button_pos)
{
    if (!view_->is_visible())
        return;

    // Calculate position below button
    ImVec2 modal_pos = ImVec2(button_pos.x, button_pos.y + 2.0f);
    view_->set_position(modal_pos);

    // Get current state from model
    std::string error_message;
    bool can_confirm = model_->can_confirm(error_message);
    bool is_new_group = model_->is_new_group();

    // Render view with current model state
    view_->render(model_->get_group_name(),
                  model_->get_channels(),
                  model_->get_selected_count(),
                  model_->get_total_count(),
                  can_confirm,
                  is_new_group);
}

bool ChannelsGroupPresenter::is_open() const
{
    return view_->is_visible();
}

// ========================================================================
// VIEW EVENT HANDLERS
// ========================================================================

void ChannelsGroupPresenter::on_group_name_changed(const std::string& new_name)
{
    // User typed in the name field - update model
    // IMPORTANT: This just changes the name, doesn't change the ID
    // So renaming an existing group will UPDATE it, not create a duplicate
    model_->set_group_name(new_name);
    // View will be updated on next render cycle
}

void ChannelsGroupPresenter::on_channel_selection_changed(size_t index, bool selected)
{
    // User clicked a channel checkbox - update model
    model_->select_channel(index, selected);
    // View will be updated on next render cycle
}

void ChannelsGroupPresenter::on_select_all_channels(bool selected)
{
    // User clicked Select All or Clear All - update model
    model_->select_all_channels(selected);
    // View will be updated on next render cycle
}

void ChannelsGroupPresenter::on_confirm()
{
    // User clicked Confirm button

    // Validate via model
    std::string error_message;
    if (!model_->can_confirm(error_message))
    {
        std::fprintf(stderr, "[ChannelsGroupPresenter] Validation failed: %s\n", error_message.c_str());
        return;
    }

    // Save to service via model
    // The model knows if this is create or update based on groupId_
    // The model will automatically call NotifyGroupsChanged() on success
    if (model_->save_channel_group())
    {
        // Create group object for callback
        models::ChannelsGroup group(model_->get_group_id(), model_->get_group_name());

        for (const auto& channel : model_->get_channels())
        {
            if (channel.selected)
            {
                group.channel_ids.push_back(channel.get_id());
            }
        }

        // Notify callback (for immediate channel configuration)
        if (on_confirm_callback_)
        {
            on_confirm_callback_(group);
        }

        // Close the modal
        close();
    }
    else
    {
        std::fprintf(stderr, "[ChannelsGroupPresenter] Failed to save group\n");
    }
}

void ChannelsGroupPresenter::on_cancel()
{
    // User closed the window or cancelled
    close();
}

void ChannelsGroupPresenter::on_delete()
{
    // User clicked Delete button
    const std::string& group_id = model_->get_group_id();

    // Delete via model (uses ID, not name!)
    // The model will automatically call NotifyGroupsChanged() on success
    if (model_->delete_channel_group())
    {
        // Notify parent that group was deleted so it can refresh tabs/UI
        if (on_delete_callback_)
        {
            on_delete_callback_(group_id);
        }

        // Close the modal
        close();
    }
    else
    {
        std::fprintf(stderr, "[ChannelsGroupPresenter] Failed to delete group: %s\n", group_id.c_str());
    }
}

// ========================================================================
// INTERNAL HELPERS
// ========================================================================

void ChannelsGroupPresenter::update_view()
{
    // This is called after model changes to ensure view reflects current state
    // In ImGui's immediate mode, the view will query model state on next render
    // So this is mostly a placeholder for future enhancements
}

void ChannelsGroupPresenter::setup_view_callbacks()
{
    // Wire up view callbacks to presenter event handlers
    IChannelsGroupViewCallbacks callbacks;

    callbacks.on_group_name_changed = [this](const std::string& name)
    {
        on_group_name_changed(name);
    };

    callbacks.on_channel_selection_changed = [this](size_t index, bool selected)
    {
        on_channel_selection_changed(index, selected);
    };

    callbacks.on_select_all_channels = [this](bool selected)
    {
        on_select_all_channels(selected);
    };

    callbacks.on_confirm = [this]()
    {
        on_confirm();
    };

    callbacks.on_cancel = [this]()
    {
        on_cancel();
    };

    callbacks.on_delete = [this]()
    {
        on_delete();
    };

    view_->set_callbacks(callbacks);
}

}  // namespace elda::views::channels_selector