#include "toast.h"

namespace elda::ui
{

Toast& Toast::instance()
{
    static Toast instance;
    return instance;
}

void Toast::show(const std::string& message, ToastType type, float duration)
{
    message_ = message;
    type_ = type;
    duration_ = duration;
    is_visible_ = true;
    start_time_ = std::chrono::steady_clock::now();
}

ImVec4 Toast::get_background_color() const
{
    // Subtle, professional dark backgrounds
    switch (type_)
    {
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

ImVec4 Toast::get_text_color() const
{
    // Subtle colored text instead of borders
    switch (type_)
    {
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

void Toast::render()
{
    if (!is_visible_)
        return;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - start_time_).count();

    if (elapsed >= duration_)
    {
        is_visible_ = false;
        return;
    }

    // Fade out
    float alpha = 1.0f;
    float time_remaining = duration_ - elapsed;
    if (time_remaining < fade_out_duration_)
    {
        alpha = time_remaining / fade_out_duration_;
    }

    // Position at bottom-right corner (more native)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    float padding = 16.0f;
    float toast_width = 320.0f;
    float toast_height = 48.0f;

    ImVec2 toast_pos(work_pos.x + work_size.x - toast_width - padding,
                     work_pos.y + work_size.y - toast_height - padding);

    ImGui::SetNextWindowPos(toast_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(toast_width, toast_height), ImGuiCond_Always);

    // Apply alpha
    ImVec4 bg_color = get_background_color();
    bg_color.w *= alpha;

    ImVec4 text_color = get_text_color();
    text_color.w *= alpha;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, alpha * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("##Toast", nullptr, flags))
    {
        // Calculate text size
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + toast_width - 24.0f);
        ImVec2 text_size = ImGui::CalcTextSize(message_.c_str(), nullptr, true, toast_width - 24.0f);
        ImGui::PopTextWrapPos();

        // Center text vertically
        float window_height = ImGui::GetWindowHeight();
        float text_offset_y = (window_height - text_size.y) * 0.5f;

        ImGui::SetCursorPosY(text_offset_y);

        // Render text
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + toast_width - 24.0f);
        ImGui::TextWrapped("%s", message_.c_str());
        ImGui::PopTextWrapPos();

        ImGui::End();
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
}

}  // namespace elda::ui
