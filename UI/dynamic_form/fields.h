#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <regex>

namespace elda::ui {

// Forward declaration
class DynamicForm;

// =============================================================================
// Select Option
// =============================================================================

struct SelectOption {
    std::string label;
    int value;
};

// =============================================================================
// Base Field
// =============================================================================

class FieldBase {
public:
    FieldBase(std::string id, std::string label);
    virtual ~FieldBase() = default;

    // Identity
    const std::string& id() const { return id_; }
    const std::string& label() const { return label_; }

    // Validation state
    bool has_error() const { return has_error_; }
    const std::string& error_text() const { return error_text_; }
    void clear_error();
    bool is_dirty() const { return dirty_; }
    void mark_dirty() { dirty_ = true; }

    // Common modifiers (return reference for chaining)
    FieldBase& required();
    FieldBase& show_when(std::function<bool(const DynamicForm&)> condition);

    // Check visibility
    bool is_visible(const DynamicForm& form) const;

    // Virtual methods
    virtual void render(float label_width) = 0;
    virtual bool validate(const DynamicForm& form);
    virtual std::string get_string_value() const = 0;

protected:
    void set_error(const std::string& msg);
    void render_label(float label_width);
    void render_error(float label_width);

    std::string id_;
    std::string label_;
    bool required_ = false;
    std::function<bool(const DynamicForm&)> visible_when_;

    bool has_error_ = false;
    std::string error_text_;
    bool dirty_ = false;
};

// =============================================================================
// Text Field
// =============================================================================

class TextField : public FieldBase {
public:
    TextField(const std::string& id, const std::string& label);

    // Modifiers
    TextField& width(float w);
    TextField& default_value(const std::string& val);
    TextField& min_length(size_t len, const std::string& msg = "");
    TextField& max_length(size_t len, const std::string& msg = "");
    TextField& pattern(const std::string& regex, const std::string& msg = "");
    TextField& custom_validate(std::function<bool(const DynamicForm&, const std::string&)> fn,
                               const std::string& msg = "Invalid value");

    // Chainable base methods
    TextField& required() { FieldBase::required(); return *this; }
    TextField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    const std::string& value() const { return value_; }
    void set_value(const std::string& val) { value_ = val; }
    std::string get_string_value() const override { return value_; }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    float width_ = 300.0f;
    std::string value_;
    size_t min_length_ = 0;
    size_t max_length_ = 0;
    std::string pattern_;
    std::string min_length_error_;
    std::string max_length_error_;
    std::string pattern_error_;
    std::function<bool(const DynamicForm&, const std::string&)> custom_validator_;
    std::string custom_error_;

    std::vector<char> buffer_;
    void sync_buffer();
};

// =============================================================================
// Password Field
// =============================================================================

class PasswordField : public FieldBase {
public:
    PasswordField(const std::string& id, const std::string& label);

    // Modifiers
    PasswordField& width(float w);
    PasswordField& min_length(size_t len, const std::string& msg = "");

    // Chainable base methods
    PasswordField& required() { FieldBase::required(); return *this; }
    PasswordField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    const std::string& value() const { return value_; }
    void set_value(const std::string& val) { value_ = val; }
    std::string get_string_value() const override { return value_; }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    float width_ = 200.0f;
    std::string value_;
    size_t min_length_ = 0;
    std::string min_length_error_;

    std::vector<char> buffer_;
    void sync_buffer();
};

// =============================================================================
// Int Field
// =============================================================================

class IntField : public FieldBase {
public:
    IntField(const std::string& id, const std::string& label);

    // Modifiers
    IntField& range(int min, int max);
    IntField& default_value(int val);
    IntField& custom_validate(std::function<bool(const DynamicForm&, int)> fn,
                              const std::string& msg = "Invalid value");

    // Chainable base methods
    IntField& required() { FieldBase::required(); return *this; }
    IntField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    int value() const { return value_; }
    void set_value(int val) { value_ = val; }
    std::string get_string_value() const override { return std::to_string(value_); }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    int min_ = 0;
    int max_ = 100;
    int value_ = 0;
    std::function<bool(const DynamicForm&, int)> custom_validator_;
    std::string custom_error_;
};

// =============================================================================
// Float Field
// =============================================================================

class FloatField : public FieldBase {
public:
    FloatField(const std::string& id, const std::string& label);

    // Modifiers
    FloatField& range(float min, float max);
    FloatField& default_value(float val);
    FloatField& precision(int decimal_places);

    // Chainable base methods
    FloatField& required() { FieldBase::required(); return *this; }
    FloatField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    float value() const { return value_; }
    void set_value(float val) { value_ = val; }
    std::string get_string_value() const override { return std::to_string(value_); }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    float min_ = 0.0f;
    float max_ = 100.0f;
    float value_ = 0.0f;
    int precision_ = 2;
};

// =============================================================================
// Select Field
// =============================================================================

class SelectField : public FieldBase {
public:
    SelectField(const std::string& id, const std::string& label);

    // Modifiers
    SelectField& options(const std::vector<SelectOption>& opts);
    SelectField& width(float w);
    SelectField& default_index(int idx);

    // Chainable base methods
    SelectField& required() { FieldBase::required(); return *this; }
    SelectField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    int selected_index() const { return selected_index_; }
    int selected_value() const;
    void set_index(int idx) { selected_index_ = idx; }
    std::string get_string_value() const override { return std::to_string(selected_index_); }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    std::vector<SelectOption> options_;
    float width_ = 200.0f;
    int selected_index_ = 0;
};

// =============================================================================
// Multiline Field
// =============================================================================

class MultilineField : public FieldBase {
public:
    MultilineField(const std::string& id, const std::string& label);

    // Modifiers
    MultilineField& size(float width, float height);
    MultilineField& default_value(const std::string& val);
    MultilineField& max_length(size_t len, const std::string& msg = "");

    // Chainable base methods
    MultilineField& required() { FieldBase::required(); return *this; }
    MultilineField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    const std::string& value() const { return value_; }
    void set_value(const std::string& val) { value_ = val; }
    std::string get_string_value() const override { return value_; }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    float width_ = 300.0f;
    float height_ = 100.0f;
    std::string value_;
    size_t max_length_ = 0;
    std::string max_length_error_;

    std::vector<char> buffer_;
    void sync_buffer();
};

// =============================================================================
// Checkbox Field
// =============================================================================

class CheckboxField : public FieldBase {
public:
    CheckboxField(const std::string& id, const std::string& label);

    // Modifiers
    CheckboxField& default_value(bool val);

    // Chainable base methods
    CheckboxField& required() { FieldBase::required(); return *this; }
    CheckboxField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    bool value() const { return value_; }
    void set_value(bool val) { value_ = val; }
    std::string get_string_value() const override { return value_ ? "true" : "false"; }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    bool value_ = false;
};

// =============================================================================
// Section Field (Title/Header)
// =============================================================================

class SectionField : public FieldBase {
public:
    SectionField(const std::string& id, const std::string& label);

    // Chainable base methods
    SectionField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // No value
    std::string get_string_value() const override { return ""; }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;
};

// =============================================================================
// Card Option
// =============================================================================

struct CardOption {
    std::string label;
    std::string description;
    int value;
};

// =============================================================================
// Card Select Field
// =============================================================================

class CardSelectField : public FieldBase {
public:
    CardSelectField(const std::string& id, const std::string& label);

    // Modifiers
    CardSelectField& options(const std::vector<CardOption>& opts);
    CardSelectField& card_size(float width, float height);
    CardSelectField& spacing(float gap);
    CardSelectField& default_index(int idx);

    // Chainable base methods
    CardSelectField& required() { FieldBase::required(); return *this; }
    CardSelectField& show_when(std::function<bool(const DynamicForm&)> cond) {
        FieldBase::show_when(std::move(cond)); return *this;
    }

    // Value access
    int selected_index() const { return selected_index_; }
    int selected_value() const;
    void set_index(int idx) { selected_index_ = idx; }
    std::string get_string_value() const override { return std::to_string(selected_index_); }

    // Overrides
    void render(float label_width) override;
    bool validate(const DynamicForm& form) override;

private:
    std::vector<CardOption> options_;
    float card_width_ = 140.0f;
    float card_height_ = 80.0f;
    float spacing_ = 16.0f;
    int selected_index_ = 0;
};

} // namespace elda::ui