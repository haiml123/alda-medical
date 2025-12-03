#include "dynamic_form.h"
#include <algorithm>

namespace elda::ui {

// =============================================================================
// Add Fields
// =============================================================================

template<typename T>
T& DynamicForm::add_field(const std::string& id, const std::string& label) {
    auto field = std::make_unique<T>(id, label);
    T& ref = *field;
    fields_.push_back(std::move(field));
    return ref;
}

SectionField& DynamicForm::add_section(const std::string& id, const std::string& title) {
    return add_field<SectionField>(id, title);
}

TextField& DynamicForm::add_text(const std::string& id, const std::string& label) {
    return add_field<TextField>(id, label);
}

IntField& DynamicForm::add_int(const std::string& id, const std::string& label) {
    return add_field<IntField>(id, label);
}

FloatField& DynamicForm::add_float(const std::string& id, const std::string& label) {
    return add_field<FloatField>(id, label);
}

SelectField& DynamicForm::add_select(const std::string& id, const std::string& label) {
    return add_field<SelectField>(id, label);
}

MultilineField& DynamicForm::add_multiline(const std::string& id, const std::string& label) {
    return add_field<MultilineField>(id, label);
}

CheckboxField& DynamicForm::add_checkbox(const std::string& id, const std::string& label) {
    return add_field<CheckboxField>(id, label);
}

CardSelectField& DynamicForm::add_card_select(const std::string& id, const std::string& label) {
    return add_field<CardSelectField>(id, label);
}

PasswordField& DynamicForm::add_password(const std::string& id, const std::string& label) {
    return add_field<PasswordField>(id, label);
}

// =============================================================================
// Get Values
// =============================================================================

std::string DynamicForm::get_string(const std::string& id) const {
    if (auto* field = dynamic_cast<const TextField*>(get_field(id))) {
        return field->value();
    }
    if (auto* field = dynamic_cast<const PasswordField*>(get_field(id))) {
        return field->value();
    }
    if (auto* field = dynamic_cast<const MultilineField*>(get_field(id))) {
        return field->value();
    }
    return "";
}

int DynamicForm::get_int(const std::string& id) const {
    if (auto* field = dynamic_cast<const IntField*>(get_field(id))) {
        return field->value();
    }
    if (auto* field = dynamic_cast<const SelectField*>(get_field(id))) {
        return field->selected_index();
    }
    if (auto* field = dynamic_cast<const CardSelectField*>(get_field(id))) {
        return field->selected_index();
    }
    return 0;
}

float DynamicForm::get_float(const std::string& id) const {
    if (auto* field = dynamic_cast<const FloatField*>(get_field(id))) {
        return field->value();
    }
    return 0.0f;
}

bool DynamicForm::get_bool(const std::string& id) const {
    if (auto* field = dynamic_cast<const CheckboxField*>(get_field(id))) {
        return field->value();
    }
    return false;
}

int DynamicForm::get_selected_index(const std::string& id) const {
    if (auto* field = dynamic_cast<const SelectField*>(get_field(id))) {
        return field->selected_index();
    }
    if (auto* field = dynamic_cast<const CardSelectField*>(get_field(id))) {
        return field->selected_index();
    }
    return -1;
}

int DynamicForm::get_selected_value(const std::string& id) const {
    if (auto* field = dynamic_cast<const SelectField*>(get_field(id))) {
        return field->selected_value();
    }
    if (auto* field = dynamic_cast<const CardSelectField*>(get_field(id))) {
        return field->selected_value();
    }
    return -1;
}

// =============================================================================
// Set Values
// =============================================================================

void DynamicForm::set_string(const std::string& id, const std::string& value) {
    if (auto* field = dynamic_cast<TextField*>(get_field(id))) {
        field->set_value(value);
    } else if (auto* field = dynamic_cast<PasswordField*>(get_field(id))) {
        field->set_value(value);
    } else if (auto* field = dynamic_cast<MultilineField*>(get_field(id))) {
        field->set_value(value);
    }
}

// =============================================================================
// Validation
// =============================================================================

bool DynamicForm::validate() {
    is_valid_ = true;

    for (auto& field : fields_) {
        if (!field->is_visible(*this)) {
            field->clear_error();
            continue;
        }

        if (!field->validate(*this)) {
            is_valid_ = false;
        }
    }

    return is_valid_;
}

std::string DynamicForm::get_first_error() const {
    for (const auto& field : fields_) {
        if (field->has_error()) {
            return field->error_text();
        }
    }
    return "";
}

void DynamicForm::mark_all_dirty() {
    for (auto& field : fields_) {
        field->mark_dirty();
    }
}

// =============================================================================
// Layout
// =============================================================================

void DynamicForm::set_layout(const std::vector<std::vector<std::string>>& layout) {
    layout_ = layout;
}

// =============================================================================
// Rendering
// =============================================================================

void DynamicForm::render(float label_width) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.11f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.13f, 0.14f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.13f, 0.14f, 0.16f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (layout_.empty()) {
        render_default(label_width);
    } else {
        render_with_layout(label_width);
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void DynamicForm::render_default(float label_width) {
    for (auto& field : fields_) {
        if (!field->is_visible(*this)) {
            continue;
        }

        field->render(label_width);
        ImGui::Spacing();
    }
}

void DynamicForm::render_with_layout(float label_width) {
    for (const auto& row : layout_) {
        // Single field row
        if (row.size() == 1) {
            if (!row[0].empty()) {
                if (auto* field = get_field(row[0])) {
                    if (field->is_visible(*this)) {
                        field->render(label_width);
                    }
                }
            }
            ImGui::Spacing();
            continue;
        }

        // Multi-column row
        ImGui::Columns(static_cast<int>(row.size()), nullptr, false);

        for (size_t i = 0; i < row.size(); i++) {
            if (!row[i].empty()) {
                if (auto* field = get_field(row[i])) {
                    if (field->is_visible(*this)) {
                        field->render(label_width);
                    }
                }
            }
            if (i < row.size() - 1) {
                ImGui::NextColumn();
            }
        }

        ImGui::Columns(1);
        ImGui::Spacing();
    }
}

// =============================================================================
// Utility
// =============================================================================

void DynamicForm::clear() {
    fields_.clear();
    layout_.clear();
    is_valid_ = false;
}

bool DynamicForm::is_field_visible(const std::string& id) const {
    if (const auto* field = get_field(id)) {
        return field->is_visible(*this);
    }
    return false;
}

FieldBase* DynamicForm::get_field(const std::string& id) {
    for (auto& field : fields_) {
        if (field->id() == id) {
            return field.get();
        }
    }
    return nullptr;
}

const FieldBase* DynamicForm::get_field(const std::string& id) const {
    for (const auto& field : fields_) {
        if (field->id() == id) {
            return field.get();
        }
    }
    return nullptr;
}

} // namespace elda::ui