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
        static Toast& instance();

        void show(const std::string& message,
                  ToastType type = ToastType::Info,
                  float duration = 3.0f);
        void render();
        bool is_visible() const { return is_visible_; }

    private:
        Toast() = default;

        bool is_visible_ = false;
        std::string message_;
        ToastType type_ = ToastType::Info;
        std::chrono::steady_clock::time_point start_time_;
        float duration_ = 3.0f;
        float fade_out_duration_ = 0.3f;

        ImVec4 get_background_color() const;
        ImVec4 get_text_color() const;
    };

} // namespace elda::ui
