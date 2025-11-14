// src/ui/components/Toast.h
#pragma once
#include <imgui.h>
#include <string>
#include <chrono>

namespace elda::ui {

    enum class ToastType {
        Info,
        Success,
        Warning,
        Error
    };

    class Toast {
    public:
        static Toast& Instance();

        void Show(const std::string& message, ToastType type = ToastType::Info, float duration = 3.0f);
        void Render();
        bool IsVisible() const { return m_isVisible; }

    private:
        Toast() = default;

        bool m_isVisible = false;
        std::string m_message;
        ToastType m_type = ToastType::Info;
        std::chrono::steady_clock::time_point m_startTime;
        float m_duration = 3.0f;
        float m_fadeOutDuration = 0.3f;

        ImVec4 GetBackgroundColor() const;
        ImVec4 GetTextColor() const;
    };

} // namespace elda::ui