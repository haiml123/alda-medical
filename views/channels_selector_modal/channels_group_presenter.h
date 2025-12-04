#pragma once

#include "../../core/app_state_manager.h"
#include "channels_group_model.h"
#include "channels_group_view.h"

#include <functional>
#include <memory>

namespace elda::views::channels_selector
{

/**
 * Presenter: Mediates between Model and View
 * - Responds to View events (user actions)
 * - Updates Model based on user input
 * - Updates View based on Model state
 * - Contains presentation logic
 *
 */
class ChannelsGroupPresenter
{
  public:
    using OnConfirmCallback = std::function<void(const models::ChannelsGroup&)>;
    using OnDeleteCallback = std::function<void(const std::string&)>;
    using OnGroupsChangedCallback = std::function<void()>;

    explicit ChannelsGroupPresenter(elda::AppStateManager& state_manager);
    ~ChannelsGroupPresenter() = default;

    // ========================================================================
    // PUBLIC API - Called by Application
    // ========================================================================

    /**
     * Open the modal with channels from a specific group (by ID)
     * @param groupId ID of the group to load (empty = new group)
     * @param callback Callback when user confirms
     * @param deleteCallback Callback when user deletes (optional)
     */
    void open(const std::string& group_id = "",
              OnConfirmCallback callback = nullptr,
              OnDeleteCallback delete_callback = nullptr);

    /**
     * Set callback to be invoked whenever groups list changes
     * This allows parent to refresh its view of available groups
     * @param callback Function to call when groups need refresh
     */
    void set_on_groups_changed_callback(OnGroupsChangedCallback callback);

    /**
     * Close the modal
     */
    void close();

    /**
     * Render the modal (call every frame)
     * @param button_pos Position to place the modal (typically below a button)
     */
    void render(ImVec2 button_pos);

    /**
     * Check if modal is currently open
     */
    bool is_open() const;

  private:
    // ========================================================================
    // VIEW EVENT HANDLERS - Respond to user actions
    // ========================================================================

    void on_group_name_changed(const std::string& new_name);
    void on_channel_selection_changed(size_t index, bool selected);
    void on_select_all_channels(bool selected);
    void on_confirm();
    void on_cancel();
    void on_delete();

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================

    void update_view();
    void setup_view_callbacks();

    // ========================================================================
    // MEMBERS
    // ========================================================================

    std::unique_ptr<ChannelsGroupModel> model_;
    std::unique_ptr<ChannelsGroupView> view_;
    OnConfirmCallback on_confirm_callback_;
    OnDeleteCallback on_delete_callback_;
};

}  // namespace elda::views::channels_selector