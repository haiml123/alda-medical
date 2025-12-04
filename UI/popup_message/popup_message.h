#pragma once
#include <functional>
#include <imgui.h>
#include <string>

namespace elda::ui
{

class PopupMessage
{
  public:
    static PopupMessage& instance();

    void show(const std::string& title,
              const std::string& message,
              std::function<void()> on_confirm,
              std::function<void()> on_cancel = nullptr);

    void render();
    bool is_open() const
    {
        return is_open_;
    }

  private:
    PopupMessage() = default;

    bool is_open_ = false;
    bool just_opened_ = false;  // Track if we need to call OpenPopup
    std::string title_;
    std::string message_;
    std::function<void()> on_confirm_;
    std::function<void()> on_cancel_;
};

}  // namespace elda::ui
