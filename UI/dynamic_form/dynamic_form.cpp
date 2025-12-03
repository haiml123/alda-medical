#include "dynamic_form.h"

namespace elda::ui {

DynamicForm::DynamicForm(FormConfig config) : config_(std::move(config)) {}

void DynamicForm::set_fields(std::vector<FormField> fields) {
    for (auto& field : fields) {
        normalize_field_value(field);
    }
    fields_ = std::move(fields);
}

void DynamicForm::set_on_submit(
    std::function<void(const std::unordered_map<std::string, FieldValue>&)> callback) {
    on_submit_ = std::move(callback);
}

FieldValue DynamicForm::default_value_for(FieldType type) {
    switch (type) {
        case FieldType::Integer:
            return 0;
        case FieldType::Float:
            return 0.0f;
        case FieldType::Checkbox:
            return false;
        case FieldType::Dropdown:
            return std::string{};
        case FieldType::Text:
        default:
            return std::string{};
    }
}

void DynamicForm::normalize_field_value(FormField& field) const {
    const auto fallback = default_value_for(field.type);
    if (field.value.index() != fallback.index()) {
        field.value = fallback;
    }
}

bool DynamicForm::render() {
    bool submitted = false;
    errors_.clear();

    if (config_.show_border) {
        ImGui::BeginChild("DynamicFormContainer", ImVec2(0, 0), true);
    }

    if (!config_.title.empty()) {
        ImGui::TextUnformatted(config_.title.c_str());
        ImGui::Spacing();
    }

    for (auto& field : fields_) {
        ImGui::PushID(field.id.c_str());
        const bool changed = render_field(field);
        ImGui::PopID();

        if (changed && field.on_change) {
            field.on_change(field.id, field.value);
        }

        ImGui::Dummy(ImVec2(0.0f, config_.spacing));
    }

    if (!errors_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.26f, 0.21f, 1.0f));
        for (const auto& error : errors_) {
            ImGui::TextUnformatted(error.c_str());
        }
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0.0f, config_.spacing));
    }

    if (ImGui::Button(config_.submit_label.c_str(), ImVec2(-1, 36))) {
        if (validate()) {
            submitted = true;
            if (on_submit_) {
                on_submit_(collect_values());
            }
        }
    }

    if (config_.show_border) {
        ImGui::EndChild();
    }

    return submitted;
}

bool DynamicForm::render_field(FormField& field) {
    bool changed = false;

    const auto label = field.label + (field.required ? " *" : "");
    ImGui::TextUnformatted(label.c_str());

    const float width = ImGui::GetContentRegionAvail().x;

    switch (field.type) {
        case FieldType::Text: {
            auto& value = std::get<std::string>(field.value);
            ImGui::SetNextItemWidth(width);
            changed = ImGui::InputTextWithHint("##text", field.placeholder.c_str(), &value);
            break;
        }
        case FieldType::Integer: {
            auto& value = std::get<int>(field.value);
            ImGui::SetNextItemWidth(width);
            changed = ImGui::InputInt("##int", &value);
            break;
        }
        case FieldType::Float: {
            auto& value = std::get<float>(field.value);
            ImGui::SetNextItemWidth(width);
            changed = ImGui::InputFloat("##float", &value, 0.0f, 0.0f, "%.3f");
            break;
        }
        case FieldType::Checkbox: {
            auto& value = std::get<bool>(field.value);
            changed = ImGui::Checkbox("##checkbox", &value);
            break;
        }
        case FieldType::Dropdown: {
            auto& current_value = std::get<std::string>(field.value);
            const char* preview = current_value.empty() ? field.placeholder.c_str() : current_value.c_str();
            if (ImGui::BeginCombo("##dropdown", preview)) {
                for (const auto& option : field.options) {
                    const bool selected = (current_value == option);
                    if (ImGui::Selectable(option.c_str(), selected)) {
                        current_value = option;
                        changed = true;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            break;
        }
    }

    if (!field.help_text.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.76f, 0.85f, 1.0f));
        ImGui::TextWrapped("%s", field.help_text.c_str());
        ImGui::PopStyleColor();
    }

    return changed;
}

bool DynamicForm::validate() {
    errors_.clear();

    for (const auto& field : fields_) {
        if (!field.required) {
            continue;
        }

        switch (field.type) {
            case FieldType::Text:
            case FieldType::Dropdown: {
                const auto& value = std::get<std::string>(field.value);
                if (value.empty()) {
                    errors_.push_back(field.label + " is required.");
                }
                break;
            }
            case FieldType::Integer: {
                // Integer fields are considered required only if they differ from the default
                if (std::get<int>(field.value) == 0) {
                    errors_.push_back(field.label + " is required.");
                }
                break;
            }
            case FieldType::Float: {
                if (std::get<float>(field.value) == 0.0f) {
                    errors_.push_back(field.label + " is required.");
                }
                break;
            }
            case FieldType::Checkbox: {
                if (!std::get<bool>(field.value)) {
                    errors_.push_back(field.label + " must be checked.");
                }
                break;
            }
        }
    }

    return errors_.empty();
}

std::unordered_map<std::string, FieldValue> DynamicForm::collect_values() const {
    std::unordered_map<std::string, FieldValue> collected;
    collected.reserve(fields_.size());

    for (const auto& field : fields_) {
        collected[field.id] = field.value;
    }

    return collected;
}

} // namespace elda::ui
