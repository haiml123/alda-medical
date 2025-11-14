#include "popup_message.h"

namespace elda::ui {

PopupMessage& PopupMessage::Instance() {
    static PopupMessage instance;
    return instance;
}

void PopupMessage::Show(const std::string& title,
                        const std::string& message,
                        std::function<void()> onConfirm,
                        std::function<void()> onCancel) {
    m_title = title;
    m_message = message;
    m_onConfirm = onConfirm;
    m_onCancel = onCancel;
    m_isOpen = true;
    m_justOpened = true;
}

void PopupMessage::Render() {
    if (!m_isOpen) return;

    if (m_justOpened) {
        ImGui::OpenPopup(m_title.c_str());
        m_justOpened = false;
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

    if (ImGui::BeginPopupModal(m_title.c_str(), nullptr,
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove)) {

        // Message text
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 410.0f);
        ImGui::TextWrapped("%s", m_message.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Spacing();

        // Buttons
        float buttonWidth = 120.0f;
        float buttonHeight = 32.0f;
        float spacing = 12.0f;
        float totalWidth = (buttonWidth * 2) + spacing;
        float offsetX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

        // Cancel button (gray)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, buttonHeight))) {
            if (m_onCancel) {
                m_onCancel();
            }
            m_isOpen = false;
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

        if (ImGui::Button("OK", ImVec2(buttonWidth, buttonHeight))) {
            if (m_onConfirm) {
                m_onConfirm();
            }
            m_isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::EndPopup();
    } else {
        m_isOpen = false;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

} // namespace elda::ui