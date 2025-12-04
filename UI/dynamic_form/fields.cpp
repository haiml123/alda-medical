#include "fields.h"

#include "dynamic_form.h"

#include <algorithm>
#include <cstring>

namespace elda::ui
{

// =============================================================================
// Base Field
// =============================================================================

FieldBase::FieldBase(std::string id, std::string label) : id_(std::move(id)), label_(std::move(label))
{
}

void FieldBase::clear_error()
{
    has_error_ = false;
    error_text_.clear();
}

FieldBase& FieldBase::required()
{
    required_ = true;
    return *this;
}

FieldBase& FieldBase::show_when(std::function<bool(const DynamicForm&)> condition)
{
    visible_when_ = std::move(condition);
    return *this;
}

bool FieldBase::is_visible(const DynamicForm& form) const
{
    if (!visible_when_)
        return true;
    return visible_when_(form);
}

bool FieldBase::validate(const DynamicForm& /*form*/)
{
    clear_error();

    if (required_)
    {
        std::string val = get_string_value();
        if (val.empty() || val == "0")
        {
            set_error(label_ + " is required");
            return false;
        }
    }

    return true;
}

void FieldBase::set_error(const std::string& msg)
{
    has_error_ = true;
    error_text_ = msg;
}

void FieldBase::render_label(float label_width)
{
    ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.68f, 0.72f, 1.0f));
    ImGui::Text("%s", label_.c_str());
    ImGui::PopStyleColor();
    ImGui::SameLine(label_width);
}

void FieldBase::render_error(float label_width)
{
    if (has_error_ && dirty_)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + label_width);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
        ImGui::Text("* %s", error_text_.c_str());
        ImGui::PopStyleColor();
    }
}

// =============================================================================
// Text Field
// =============================================================================

TextField::TextField(const std::string& id, const std::string& label) : FieldBase(id, label), buffer_(256, '\0')
{
}

TextField& TextField::width(float w)
{
    width_ = w;
    return *this;
}

TextField& TextField::default_value(const std::string& val)
{
    value_ = val;
    return *this;
}

TextField& TextField::min_length(size_t len, const std::string& msg)
{
    min_length_ = len;
    min_length_error_ = msg.empty() ? "Minimum " + std::to_string(len) + " characters" : msg;
    return *this;
}

TextField& TextField::max_length(size_t len, const std::string& msg)
{
    max_length_ = len;
    max_length_error_ = msg.empty() ? "Maximum " + std::to_string(len) + " characters" : msg;
    return *this;
}

TextField& TextField::pattern(const std::string& regex, const std::string& msg)
{
    pattern_ = regex;
    pattern_error_ = msg.empty() ? "Invalid format" : msg;
    return *this;
}

TextField& TextField::custom_validate(std::function<bool(const DynamicForm&, const std::string&)> fn,
                                      const std::string& msg)
{
    custom_validator_ = std::move(fn);
    custom_error_ = msg;
    return *this;
}

void TextField::sync_buffer()
{
    if (buffer_.size() < 256)
        buffer_.resize(256, '\0');
    if (std::strncmp(buffer_.data(), value_.c_str(), buffer_.size()) != 0)
    {
        std::strncpy(buffer_.data(), value_.c_str(), buffer_.size() - 1);
    }
}

void TextField::render(float label_width)
{
    render_label(label_width);

    ImGui::PushItemWidth(width_);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    sync_buffer();
    if (ImGui::InputText(("##" + id_).c_str(), buffer_.data(), buffer_.size()))
    {
        value_ = buffer_.data();
        dirty_ = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    render_error(label_width);
}

bool TextField::validate(const DynamicForm& form)
{
    if (!FieldBase::validate(form))
        return false;

    if (min_length_ > 0 && value_.length() < min_length_)
    {
        set_error(min_length_error_);
        return false;
    }

    if (max_length_ > 0 && value_.length() > max_length_)
    {
        set_error(max_length_error_);
        return false;
    }

    if (!pattern_.empty() && !value_.empty())
    {
        try
        {
            std::regex re(pattern_);
            if (!std::regex_match(value_, re))
            {
                set_error(pattern_error_);
                return false;
            }
        }
        catch (const std::regex_error&)
        {
            // Invalid regex - skip
        }
    }

    if (custom_validator_ && !custom_validator_(form, value_))
    {
        set_error(custom_error_.empty() ? "Invalid value" : custom_error_);
        return false;
    }

    return true;
}

// =============================================================================
// Password Field
// =============================================================================

PasswordField::PasswordField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
    buffer_.resize(256, '\0');
}

PasswordField& PasswordField::width(float w)
{
    width_ = w;
    return *this;
}

PasswordField& PasswordField::min_length(size_t len, const std::string& msg)
{
    min_length_ = len;
    min_length_error_ = msg.empty() ? "At least " + std::to_string(len) + " characters" : msg;
    return *this;
}

void PasswordField::sync_buffer()
{
    if (std::strncmp(buffer_.data(), value_.c_str(), buffer_.size()) != 0)
    {
        std::strncpy(buffer_.data(), value_.c_str(), buffer_.size() - 1);
    }
}

void PasswordField::render(float label_width)
{
    render_label(label_width);

    ImGui::PushItemWidth(width_);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    sync_buffer();
    if (ImGui::InputText(("##" + id_).c_str(), buffer_.data(), buffer_.size(), ImGuiInputTextFlags_Password))
    {
        value_ = buffer_.data();
        dirty_ = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    render_error(label_width);
}

bool PasswordField::validate(const DynamicForm& /*form*/)
{
    clear_error();

    if (required_ && value_.empty())
    {
        set_error(label_ + " is required");
        return false;
    }

    if (min_length_ > 0 && value_.length() < min_length_)
    {
        set_error(min_length_error_);
        return false;
    }

    return true;
}

// =============================================================================
// Int Field
// =============================================================================

IntField::IntField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

IntField& IntField::range(int min, int max)
{
    min_ = min;
    max_ = max;
    return *this;
}

IntField& IntField::default_value(const int val)
{
    value_ = val;
    return *this;
}

IntField& IntField::custom_validate(std::function<bool(const DynamicForm&, int)> fn, const std::string& msg)
{
    custom_validator_ = std::move(fn);
    custom_error_ = msg;
    return *this;
}

void IntField::render(const float label_width)
{
    render_label(label_width);

    ImGui::PushItemWidth(100.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    if (ImGui::InputInt(("##" + id_).c_str(), &value_, 1, 10))
    {
        value_ = std::clamp(value_, min_, max_);
        dirty_ = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    render_error(label_width);
}

bool IntField::validate(const DynamicForm& form)
{
    if (!FieldBase::validate(form))
        return false;

    if (custom_validator_ && !custom_validator_(form, value_))
    {
        set_error(custom_error_.empty() ? "Invalid value" : custom_error_);
        return false;
    }

    return true;
}

// =============================================================================
// Float Field
// =============================================================================

FloatField::FloatField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

FloatField& FloatField::range(float min, float max)
{
    min_ = min;
    max_ = max;
    return *this;
}

FloatField& FloatField::default_value(float val)
{
    value_ = val;
    return *this;
}

FloatField& FloatField::precision(int decimal_places)
{
    precision_ = decimal_places;
    return *this;
}

void FloatField::render(float label_width)
{
    render_label(label_width);

    ImGui::PushItemWidth(120.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    std::string fmt = "%." + std::to_string(precision_) + "f";
    if (ImGui::InputFloat(("##" + id_).c_str(), &value_, 0.1f, 1.0f, fmt.c_str()))
    {
        value_ = std::clamp(value_, min_, max_);
        dirty_ = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    render_error(label_width);
}

bool FloatField::validate(const DynamicForm& form)
{
    return FieldBase::validate(form);
}

// =============================================================================
// Select Field
// =============================================================================

SelectField::SelectField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

SelectField& SelectField::options(const std::vector<SelectOption>& opts)
{
    options_ = opts;
    return *this;
}

SelectField& SelectField::width(float w)
{
    width_ = w;
    return *this;
}

SelectField& SelectField::default_index(int idx)
{
    selected_index_ = idx;
    return *this;
}

int SelectField::selected_value() const
{
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(options_.size()))
    {
        return options_[selected_index_].value;
    }
    return -1;
}

void SelectField::render(float label_width)
{
    render_label(label_width);

    ImGui::PushItemWidth(width_);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    const char* preview = (selected_index_ >= 0 && selected_index_ < static_cast<int>(options_.size()))
                              ? options_[selected_index_].label.c_str()
                              : "Select...";

    if (ImGui::BeginCombo(("##" + id_).c_str(), preview))
    {
        for (int i = 0; i < static_cast<int>(options_.size()); i++)
        {
            bool selected = (selected_index_ == i);
            if (ImGui::Selectable(options_[i].label.c_str(), selected))
            {
                selected_index_ = i;
                dirty_ = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    render_error(label_width);
}

bool SelectField::validate(const DynamicForm& form)
{
    clear_error();

    if (required_ && selected_index_ < 0)
    {
        set_error(label_ + " is required");
        return false;
    }

    return true;
}

// =============================================================================
// Multiline Field
// =============================================================================

MultilineField::MultilineField(const std::string& id, const std::string& label)
    : FieldBase(id, label), buffer_(1024, '\0')
{
}

MultilineField& MultilineField::size(float width, float height)
{
    width_ = width;
    height_ = height;
    return *this;
}

MultilineField& MultilineField::default_value(const std::string& val)
{
    value_ = val;
    return *this;
}

MultilineField& MultilineField::max_length(size_t len, const std::string& msg)
{
    max_length_ = len;
    max_length_error_ = msg.empty() ? "Maximum " + std::to_string(len) + " characters" : msg;
    return *this;
}

void MultilineField::sync_buffer()
{
    if (buffer_.size() < 1024)
        buffer_.resize(1024, '\0');
    if (std::strncmp(buffer_.data(), value_.c_str(), buffer_.size()) != 0)
    {
        std::strncpy(buffer_.data(), value_.c_str(), buffer_.size() - 1);
    }
}

void MultilineField::render(float label_width)
{
    render_label(label_width);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));

    sync_buffer();
    if (ImGui::InputTextMultiline(("##" + id_).c_str(), buffer_.data(), buffer_.size(), ImVec2(width_, height_)))
    {
        value_ = buffer_.data();
        dirty_ = true;
    }

    ImGui::PopStyleColor();

    render_error(label_width);
}

bool MultilineField::validate(const DynamicForm& form)
{
    if (!FieldBase::validate(form))
        return false;

    if (max_length_ > 0 && value_.length() > max_length_)
    {
        set_error(max_length_error_);
        return false;
    }

    return true;
}

// =============================================================================
// Checkbox Field
// =============================================================================

CheckboxField::CheckboxField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

CheckboxField& CheckboxField::default_value(bool val)
{
    value_ = val;
    return *this;
}

void CheckboxField::render(float label_width)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + label_width);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    if (ImGui::Checkbox((label_ + "##" + id_).c_str(), &value_))
    {
        dirty_ = true;
    }
    ImGui::PopStyleColor();

    render_error(label_width);
}

bool CheckboxField::validate(const DynamicForm& form)
{
    clear_error();

    if (required_ && !value_)
    {
        set_error(label_ + " must be checked");
        return false;
    }

    return true;
}

// =============================================================================
// Section Field
// =============================================================================

SectionField::SectionField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

void SectionField::render(float /*label_width*/)
{
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("%s", label_.c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

bool SectionField::validate(const DynamicForm& /*form*/)
{
    return true;  // Sections don't validate
}

// =============================================================================
// Card Select Field
// =============================================================================

CardSelectField::CardSelectField(const std::string& id, const std::string& label) : FieldBase(id, label)
{
}

CardSelectField& CardSelectField::options(const std::vector<CardOption>& opts)
{
    options_ = opts;
    return *this;
}

CardSelectField& CardSelectField::card_size(float width, float height)
{
    card_width_ = width;
    card_height_ = height;
    return *this;
}

CardSelectField& CardSelectField::spacing(float gap)
{
    spacing_ = gap;
    return *this;
}

CardSelectField& CardSelectField::default_index(int idx)
{
    selected_index_ = idx;
    return *this;
}

int CardSelectField::selected_value() const
{
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(options_.size()))
    {
        return options_[selected_index_].value;
    }
    return -1;
}

void CardSelectField::render(float label_width)
{
    // Label
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.87f, 0.90f, 1.0f));
    ImGui::Text("%s", label_.c_str());
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Render cards in a row
    for (int i = 0; i < static_cast<int>(options_.size()); i++)
    {
        if (i > 0)
        {
            ImGui::SameLine(0, spacing_);
        }

        bool is_selected = (selected_index_ == i);
        const auto& opt = options_[i];

        // Colors
        ImVec4 bg_color = is_selected ? ImVec4(0.20f, 0.65f, 0.70f, 0.3f) : ImVec4(0.12f, 0.13f, 0.15f, 1.0f);
        ImVec4 border_color = is_selected ? ImVec4(0.30f, 0.90f, 0.95f, 1.0f) : ImVec4(1, 1, 1, 0.1f);
        ImVec4 hover_color = is_selected ? bg_color : ImVec4(0.15f, 0.16f, 0.18f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, bg_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_color);
        ImGui::PushStyleColor(ImGuiCol_Border, border_color);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, is_selected ? 2.0f : 1.0f);

        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

        if (ImGui::Button(("##card_" + id_ + "_" + std::to_string(i)).c_str(), ImVec2(card_width_, card_height_)))
        {
            selected_index_ = i;
            dirty_ = true;
        }

        // Draw label and description
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImU32 label_color = is_selected ? IM_COL32(77, 230, 235, 255) : IM_COL32(217, 220, 226, 255);
        ImU32 desc_color = IM_COL32(166, 170, 179, 255);

        draw_list->AddText(ImVec2(cursor_pos.x + 12, cursor_pos.y + 16), label_color, opt.label.c_str());

        draw_list->AddText(ImVec2(cursor_pos.x + 12, cursor_pos.y + 40), desc_color, opt.description.c_str());

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }

    render_error(label_width);
}

bool CardSelectField::validate(const DynamicForm& form)
{
    clear_error();

    if (required_ && selected_index_ < 0)
    {
        set_error(label_ + " is required");
        return false;
    }

    return true;
}

}  // namespace elda::ui