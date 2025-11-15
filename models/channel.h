#pragma once
#include "base_model.h"
#include <string>

namespace elda::models {

    struct Channel : public BaseModel {
        std::string name;
        std::string color;
        bool selected;

        int amplifier_channel;
        std::string signal_type;
        double sensor_gain;
        double sensor_offset;

        bool filtered;
        double high_pass_cutoff;
        double low_pass_cutoff;

        float impedance_x;
        float impedance_y;

        Channel()
            : BaseModel()
            , color("#FFFFFF")
            , selected(false)
            , amplifier_channel(-1)
            , signal_type("EEG")
            , sensor_gain(1.0)
            , sensor_offset(0.0)
            , filtered(false)
            , high_pass_cutoff(0.0)
            , low_pass_cutoff(0.0)
            , impedance_x(0.0f)
            , impedance_y(0.0f) {}

        Channel(const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel()
            , name(name_)
            , color(color_)
            , selected(false)
            , amplifier_channel(-1)
            , signal_type("EEG")
            , sensor_gain(1.0)
            , sensor_offset(0.0)
            , filtered(false)
            , high_pass_cutoff(0.0)
            , low_pass_cutoff(0.0)
            , impedance_x(0.0f)
            , impedance_y(0.0f) {}

        Channel(const std::string& id_, const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel(id_)
            , name(name_)
            , color(color_)
            , selected(false)
            , amplifier_channel(-1)
            , signal_type("EEG")
            , sensor_gain(1.0)
            , sensor_offset(0.0)
            , filtered(false)
            , high_pass_cutoff(0.0)
            , low_pass_cutoff(0.0)
            , impedance_x(0.0f)
            , impedance_y(0.0f) {}

        void set_selected(bool is_selected) {
            if (selected != is_selected) {
                selected = is_selected;
                on_update();
            }
        }

        void set_gain(double gain) {
            if (sensor_gain != gain) {
                sensor_gain = gain;
                on_update();
            }
        }

        void set_filtering(bool enable, double high_pass = 0.0, double low_pass = 0.0) {
            filtered = enable;
            high_pass_cutoff = high_pass;
            low_pass_cutoff = low_pass;
            on_update();
        }

        void set_impedance_position(float x, float y) {
            if (impedance_x != x || impedance_y != y) {
                impedance_x = x;
                impedance_y = y;
                on_update();
            }
        }
    };

} // namespace elda::models
