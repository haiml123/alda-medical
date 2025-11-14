#pragma once
#include <imgui.h>
#include <string>
#include <functional>

namespace elda::ui {

    class PopupMessage {
    public:
        static PopupMessage& Instance();

        void Show(const std::string& title,
                  const std::string& message,
                  std::function<void()> onConfirm,
                  std::function<void()> onCancel = nullptr);

        void Render();
        bool IsOpen() const { return m_isOpen; }

    private:
        PopupMessage() = default;

        bool m_isOpen = false;
        bool m_justOpened = false;  // Track if we need to call OpenPopup
        std::string m_title;
        std::string m_message;
        std::function<void()> m_onConfirm;
        std::function<void()> m_onCancel;
    };

} // namespace elda::ui