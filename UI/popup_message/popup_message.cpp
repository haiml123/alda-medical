#include "popup_message.h"

namespace elda::ui
{

PopupMessage& PopupMessage::instance()
{
    static PopupMessage instance;
    return instance;
}

void PopupMessage::show(const std::string& title,
                        const std::string& message,
                        std::function<void()> on_confirm,
                        std::function<void()> on_cancel)
{
    title_ = title;
    message_ = message;
    on_confirm_ = on_confirm;
    on_cancel_ = on_cancel;
    is_open_ = true;
    just_opened_ = true;
}

void PopupMessage::render()
{
    if (!is_open_)
        return;

    if (just_opened_)
    {
        ImGui::OpenPopup(title_.c_str());
        just_opened_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 0), ImGuiCond_Appearing);

    // Custom colors for dark theme
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    if (ImGui::BeginPopupModal(title_.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // Message text
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 410.0f);
        ImGui::TextWrapped("%s", message_.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Spacing();

        // Buttons
        float button_width = 120.0f;
        float button_height = 32.0f;
        float spacing = 12.0f;
        float total_width = (button_width * 2) + spacing;
        float offset_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

        // Cancel button (gray)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

        if (ImGui::Button("Cancel", ImVec2(button_width, button_height)))
        {
            if (on_cancel_)
            {
                on_cancel_();
            }
            is_open_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0, spacing);

        // OK button (blue)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.48f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.56f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.40f, 0.9f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

        if (ImGui::Button("OK", ImVec2(button_width, button_height)))
        {
            if (on_confirm_)
            {
                on_confirm_();
            }
            is_open_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::EndPopup();
    }
    else
    {
        is_open_ = false;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

}  // namespace elda::ui
