#pragma once

#include "fields.h"
#include <vector>
#include <memory>

namespace elda::ui {

class DynamicForm {
public:
    DynamicForm() = default;

    // -------------------------------------------------------------------------
    // Add Fields - returns reference to field for chaining
    // -------------------------------------------------------------------------

    SectionField& add_section(const std::string& id, const std::string& title);
    TextField& add_text(const std::string& id, const std::string& label);
    IntField& add_int(const std::string& id, const std::string& label);
    FloatField& add_float(const std::string& id, const std::string& label);
    SelectField& add_select(const std::string& id, const std::string& label);
    MultilineField& add_multiline(const std::string& id, const std::string& label);
    CheckboxField& add_checkbox(const std::string& id, const std::string& label);
    CardSelectField& add_card_select(const std::string& id, const std::string& label);
    PasswordField& add_password(const std::string& id, const std::string& label);

    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------

    // Set column layout: each inner vector is a row, each string is a field id
    // Example: {{"name", "config_a"}, {"age", "config_b"}}
    void set_layout(const std::vector<std::vector<std::string>>& layout);

    // -------------------------------------------------------------------------
    // Get Values
    // -------------------------------------------------------------------------

    std::string get_string(const std::string& id) const;
    int get_int(const std::string& id) const;
    float get_float(const std::string& id) const;
    bool get_bool(const std::string& id) const;
    int get_selected_index(const std::string& id) const;
    int get_selected_value(const std::string& id) const;  // Returns value, not index

    // -------------------------------------------------------------------------
    // Set Values
    // -------------------------------------------------------------------------

    void set_string(const std::string& id, const std::string& value);

    // -------------------------------------------------------------------------
    // Validation
    // -------------------------------------------------------------------------

    bool validate();
    bool is_valid() const { return is_valid_; }
    std::string get_first_error() const;
    void mark_all_dirty();  // Mark all fields dirty to show errors

    // -------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------

    void render(float label_width = 140.0f);

    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------

    void clear();
    bool is_field_visible(const std::string& id) const;
    FieldBase* get_field(const std::string& id);
    const FieldBase* get_field(const std::string& id) const;

private:
    std::vector<std::unique_ptr<FieldBase>> fields_;
    std::vector<std::vector<std::string>> layout_;
    bool is_valid_ = false;

    template<typename T>
    T& add_field(const std::string& id, const std::string& label);

    void render_default(float label_width);
    void render_with_layout(float label_width);
};

} // namespace elda::ui