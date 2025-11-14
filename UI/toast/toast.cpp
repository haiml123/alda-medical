// src/ui/components/Toast.cpp
#include "toast.h"

namespace elda::ui {

Toast& Toast::Instance() {
    static Toast instance;
    return instance;
}

void Toast::Show(const std::string& message, ToastType type, float duration) {
    m_message = message;
    m_type = type;
    m_duration = duration;
    m_isVisible = true;
    m_startTime = std::chrono::steady_clock::now();
}

ImVec4 Toast::GetBackgroundColor() const {
    // Subtle, professional dark backgrounds
    switch (m_type) {
        case ToastType::Success:
            return ImVec4(0.16f, 0.20f, 0.17f, 0.95f);  // Dark green tint
        case ToastType::Warning:
            return ImVec4(0.22f, 0.19f, 0.15f, 0.95f);  // Dark orange tint
        case ToastType::Error:
            return ImVec4(0.22f, 0.16f, 0.16f, 0.95f);  // Dark red tint
        case ToastType::Info:
        default:
            return ImVec4(0.15f, 0.15f, 0.15f, 0.95f);  // Neutral dark
    }
}

ImVec4 Toast::GetTextColor() const {
    // Subtle colored text instead of borders
    switch (m_type) {
        case ToastType::Success:
            return ImVec4(0.4f, 0.9f, 0.5f, 1.0f);  // Light green
        case ToastType::Warning:
            return ImVec4(1.0f, 0.8f, 0.4f, 1.0f);  // Light orange
        case ToastType::Error:
            return ImVec4(1.0f, 0.5f, 0.5f, 1.0f);  // Light red
        case ToastType::Info:
        default:
            return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);  // Light gray
    }
}

void Toast::Render() {
    if (!m_isVisible) return;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_startTime).count();

    if (elapsed >= m_duration) {
        m_isVisible = false;
        return;
    }

    // Fade out
    float alpha = 1.0f;
    float timeRemaining = m_duration - elapsed;
    if (timeRemaining < m_fadeOutDuration) {
        alpha = timeRemaining / m_fadeOutDuration;
    }

    // Position at bottom-right corner (more native)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float padding = 16.0f;
    float toastWidth = 320.0f;
    float toastHeight = 48.0f;

    ImVec2 toastPos(
        workPos.x + workSize.x - toastWidth - padding,
        workPos.y + workSize.y - toastHeight - padding
    );

    ImGui::SetNextWindowPos(toastPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(toastWidth, toastHeight), ImGuiCond_Always);

    // Apply alpha
    ImVec4 bgColor = GetBackgroundColor();
    bgColor.w *= alpha;

    ImVec4 textColor = GetTextColor();
    textColor.w *= alpha;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, alpha * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("##Toast", nullptr, flags)) {
        // Calculate text size
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + toastWidth - 24.0f);
        ImVec2 textSize = ImGui::CalcTextSize(m_message.c_str(), nullptr, true, toastWidth - 24.0f);
        ImGui::PopTextWrapPos();

        // Center text vertically
        float windowHeight = ImGui::GetWindowHeight();
        float textOffsetY = (windowHeight - textSize.y) * 0.5f;

        ImGui::SetCursorPosY(textOffsetY);

        // Render text
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + toastWidth - 24.0f);
        ImGui::TextWrapped("%s", m_message.c_str());
        ImGui::PopTextWrapPos();

        ImGui::End();
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
}

} // namespace elda::ui