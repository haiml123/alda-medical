#pragma once
#include "base_model.h"
#include <string>

namespace elda::models {

    struct Channel : public BaseModel {
        std::string name;
        std::string color;
        bool selected;

        int amplifierChannel;
        std::string signalType;
        double sensorGain;
        double sensorOffset;

        bool filtered;
        double highPassCutoff;
        double lowPassCutoff;

        float impedanceX;
        float impedanceY;

        Channel()
            : BaseModel()
            , color("#FFFFFF")
            , selected(false)
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0)
            , impedanceX(0.0f)
            , impedanceY(0.0f) {}

        Channel(const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel()
            , name(name_)
            , color(color_)
            , selected(false)
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0)
            , impedanceX(0.0f)
            , impedanceY(0.0f) {}

        Channel(const std::string& id_, const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel(id_)
            , name(name_)
            , color(color_)
            , selected(false)
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0)
            , impedanceX(0.0f)
            , impedanceY(0.0f) {}

        void SetSelected(bool isSelected) {
            if (selected != isSelected) {
                selected = isSelected;
                OnUpdate();
            }
        }

        void SetGain(double gain) {
            if (sensorGain != gain) {
                sensorGain = gain;
                OnUpdate();
            }
        }

        void SetFiltering(bool enable, double highPass = 0.0, double lowPass = 0.0) {
            filtered = enable;
            highPassCutoff = highPass;
            lowPassCutoff = lowPass;
            OnUpdate();
        }

        void SetImpedancePosition(float x, float y) {
            if (impedanceX != x || impedanceY != y) {
                impedanceX = x;
                impedanceY = y;
                OnUpdate();
            }
        }
    };

} // namespace elda::models