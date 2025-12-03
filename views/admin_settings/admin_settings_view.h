#pragma once

#include "imgui.h"
#include "admin_settings_model.h"
#include "UI/screen_header//screen_header.h"
#include "UI/tabbar/tabbar.h"
#include "UI/dynamic_form/dynamic_form.h"
#include "views/admin_settings/tabs/channels_config/channels_config_model.h"
#include "views/admin_settings/tabs/channels_config/channels_config_view.h"
#include "views/admin_settings/tabs/channels_config/channels_config_presenter.h"
#include <functional>
#include <memory>

namespace elda::views::admin_settings {

struct AdminSettingsViewData {
    int active_tab = 0;
};

struct AdminSettingsViewCallbacks {
    std::function<void()> on_save;
    std::function<void()> on_close;
    std::function<void(int)> on_tab_changed;
};

class AdminSettingsView {
public:
    AdminSettingsView();

    void setup_form();
    void render(const AdminSettingsViewData& data, const AdminSettingsViewCallbacks& callbacks);
    
    elda::ui::DynamicForm& form() { return form_; }
    int get_active_tab() const { return tab_bar_.get_active_tab(); }
    
    // Access to channels config for saving
    channels_config::ChannelsConfigModel& channels_model() { return channels_model_; }

private:
    void setup_tabs();
    void render_tab_bar(const AdminSettingsViewData& data, const AdminSettingsViewCallbacks& callbacks);
    void render_content(const AdminSettingsViewData& data);
    void render_configuration_tab();
    void render_electrode_config_tab();

    elda::ui::DynamicForm form_;
    elda::ui::TabBar tab_bar_;
    
    // Channels configuration (embedded in Electrode Config tab)
    channels_config::ChannelsConfigModel channels_model_;
    channels_config::ChannelsConfigView channels_view_;
    std::unique_ptr<channels_config::ChannelsConfigPresenter> channels_presenter_;
};

} // namespace elda::views::admin_settings
