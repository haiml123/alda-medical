#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace elda::ui {

using FieldValue = std::variant<std::string, int, float, bool>;

enum class FieldType {
    Text,
    Integer,
    Float,
    Checkbox,
    Dropdown
};

struct FormField {
    std::string id;
    std::string label;
    FieldType type{FieldType::Text};
    FieldValue value{std::string{}};
    std::vector<std::string> options;
    std::string placeholder;
    std::string help_text;
    bool required{false};
    std::function<void(const std::string&, const FieldValue&)> on_change;
};

struct FormConfig {
    std::string title;
    std::string submit_label{"Save"};
    bool show_border{true};
    float spacing{8.0f};
};

/**
 * @brief Simple reusable dynamic form that renders fields from configuration
 */
class DynamicForm {
public:
    DynamicForm() = default;
    explicit DynamicForm(FormConfig config);

    void set_fields(std::vector<FormField> fields);
    std::vector<FormField>& fields() { return fields_; }
    const std::vector<FormField>& fields() const { return fields_; }

    void set_config(const FormConfig& config) { config_ = config; }
    const FormConfig& config() const { return config_; }

    void set_on_submit(std::function<void(const std::unordered_map<std::string, FieldValue>&)> callback);

    /**
     * @return true when the submit button was pressed and validation passed
     */
    bool render();

    const std::vector<std::string>& errors() const { return errors_; }

private:
    static FieldValue default_value_for(FieldType type);
    void normalize_field_value(FormField& field) const;
    bool render_field(FormField& field);
    bool validate();
    std::unordered_map<std::string, FieldValue> collect_values() const;

    FormConfig config_{};
    std::vector<FormField> fields_{};
    std::vector<std::string> errors_{};
    std::function<void(const std::unordered_map<std::string, FieldValue>&)> on_submit_;
};

} // namespace elda::ui
