#pragma once

#include "UI/dynamic_form/dynamic_form.h"
#include "imgui.h"

#include <functional>
#include <string>

namespace elda::ui
{

struct AdminLoginCallbacks
{
    std::function<void(const std::string& username, const std::string& password)> on_login;
    std::function<void()> on_cancel;
};

class AdminLoginModal
{
  public:
    // Singleton access
    static AdminLoginModal& instance()
    {
        static AdminLoginModal inst;
        return inst;
    }

    void set_callbacks(AdminLoginCallbacks callbacks)
    {
        callbacks_ = std::move(callbacks);
    }

    void handle_input()
    {
        // Detect double-F press
        if (ImGui::IsKeyPressed(ImGuiKey_F, false))
        {
            double current_time = ImGui::GetTime();
            if (current_time - last_f_press_time_ < double_press_threshold_)
            {
                open();
            }
            last_f_press_time_ = current_time;
        }

        // Close on Escape
        if (is_open_ && ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            close();
        }
    }

    void render()
    {
        if (!is_open_)
            return;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 24));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.27f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.10f, 0.11f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.10f, 0.11f, 0.12f, 1.0f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin("Admin Login", &is_open_, flags))
        {
            const float label_width = 90.0f;
            const float input_width = 240.0f;
            const float form_width = label_width + input_width;

            // Center the form
            float content_width = ImGui::GetContentRegionAvail().x;
            float form_offset = (content_width - form_width) * 0.5f;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + form_offset);
            ImGui::BeginGroup();
            form_.render(label_width);
            ImGui::EndGroup();

            // Error message
            if (!error_message_.empty())
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + form_offset);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
                ImGui::TextWrapped("%s", error_message_.c_str());
                ImGui::PopStyleColor();
            }

            ImGui::Spacing();
            ImGui::Spacing();

            // Login button aligned with form right edge
            render_button(form_offset, form_width);

            // Enter key submits
            if (ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                handle_login();
            }
        }
        ImGui::End();

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(3);
    }

    void open()
    {
        is_open_ = true;
        clear();
    }

    void close()
    {
        is_open_ = false;
        clear();
    }

    bool is_open() const
    {
        return is_open_;
    }

    void set_error(const std::string& message)
    {
        error_message_ = message;
    }

    void clear_error()
    {
        error_message_.clear();
    }

  private:
    AdminLoginModal()
    {
        setup_form();
    }

    // Prevent copying
    AdminLoginModal(const AdminLoginModal&) = delete;
    AdminLoginModal& operator=(const AdminLoginModal&) = delete;

    void setup_form()
    {
        form_.add_text("username", "Username").width(240.0f).required();

        form_.add_password("password", "Password").width(240.0f).required();
    }

    void render_button(float form_offset, float form_width)
    {
        const float button_width = 100.0f;
        const float button_height = 36.0f;

        // Position button at right edge of form
        float button_x = form_offset + form_width - button_width;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + button_x);

        // Login button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.52f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.46f, 0.90f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.42f, 0.85f, 1.0f));

        if (ImGui::Button("Login", ImVec2(button_width, button_height)))
        {
            handle_login();
        }

        ImGui::PopStyleColor(3);
    }

    void handle_login()
    {
        form_.mark_all_dirty();
        if (!form_.validate())
        {
            return;
        }

        if (callbacks_.on_login)
        {
            callbacks_.on_login(form_.get_string("username"), form_.get_string("password"));
        }
    }

    void clear()
    {
        form_.set_string("username", "");
        form_.set_string("password", "");
        error_message_.clear();
    }

    bool is_open_ = false;
    DynamicForm form_;
    std::string error_message_;
    AdminLoginCallbacks callbacks_;

    double last_f_press_time_ = 0.0;
    const double double_press_threshold_ = 0.3;  // 300ms
};

}  // namespace elda::ui