#pragma once

#include "imgui.h"
#include "user_settings_model.h"
#include "UI/screen_header/screen_header.h"
#include "UI/dynamic_form/dynamic_form.h"
#include <functional>

namespace elda::views::user_settings {

    struct UserSettingsViewData {
        bool can_proceed = false;
    };

    struct UserSettingsViewCallbacks {
        std::function<void()> on_proceed;
        std::function<void()> on_admin;
        std::function<void()> on_back;
    };

    class UserSettingsView {
    public:
        UserSettingsView() = default;

        void setup_form();
        void render(const UserSettingsViewData& data, const UserSettingsViewCallbacks& callbacks);

        elda::ui::DynamicForm& form() { return form_; }

    private:
        void render_content();

        elda::ui::DynamicForm form_;
    };

} // namespace elda::views::user_settings