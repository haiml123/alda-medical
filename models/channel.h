#pragma once
#include "base_model.h"
#include <string>
#include <string_view>

namespace elda::models {

    struct Channel final : BaseModel {
        std::string name;
        std::string color        = "#FFFFFF";
        bool        selected     = false;

        int         amplifier_channel = -1;
        std::string signal_type       = "EEG";
        double      sensor_gain       = 1.0;
        double      sensor_offset     = 0.0;

        bool   filtered         = false;
        double high_pass_cutoff = 0.0;
        double low_pass_cutoff  = 0.0;

        float impedance_x = 0.0f;
        float impedance_y = 0.0f;

        // Default ctor uses default member initializers
        Channel() = default;

        // Create channel without id (BaseModel default ctor)
        explicit Channel(const std::string_view name,
                         const std::string_view color = "#FFFFFF")
            : BaseModel()
            , name(name)
            , color(color)
        {}

        // Create channel with id
        Channel(const std::string& id,
                const std::string_view name,
                const std::string_view color = "#FFFFFF")
            : BaseModel(id)
            , name(name)
            , color(color)
        {}

        void set_selected(const bool is_selected) {
            if (selected != is_selected) {
                selected = is_selected;
                on_update();
            }
        }

        void set_gain(const double gain) {
            if (sensor_gain != gain) {
                sensor_gain = gain;
                on_update();
            }
        }

        void set_filtering(const bool enable, const double high_pass = 0.0, const double low_pass = 0.0) {
            if (filtered != enable ||
                high_pass_cutoff != high_pass ||
                low_pass_cutoff != low_pass) {
                filtered         = enable;
                high_pass_cutoff = high_pass;
                low_pass_cutoff  = low_pass;
                on_update();
            }
        }

        void set_impedance_position(const float x, const float y) {
            if (impedance_x != x || impedance_y != y) {
                impedance_x = x;
                impedance_y = y;
                on_update();
            }
        }
    };

} // namespace elda::models
