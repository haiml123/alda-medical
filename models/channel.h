#pragma once
#include "base_model.h"
#include <string>

namespace elda::models {

    struct Channel : public BaseModel {
        std::string name;
        std::string color;
        bool selected;
        std::string position;  // 10-20 system position (e.g., "Fp1", "C3", "Pz")

        int amplifierChannel;
        std::string signalType;
        double sensorGain;
        double sensorOffset;

        bool filtered;
        double highPassCutoff;
        double lowPassCutoff;

        Channel()
            : BaseModel()
            , color("#FFFFFF")
            , selected(false)
            , position("")
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0) {}

        Channel(const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel()
            , name(name_)
            , color(color_)
            , selected(false)
            , position("")
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0) {}

        Channel(const std::string& id_, const std::string& name_, const std::string& color_ = "#FFFFFF")
            : BaseModel(id_)
            , name(name_)
            , color(color_)
            , selected(false)
            , position("")
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0) {}

        // Add a setter for position
        void SetPosition(const std::string& pos) {
            if (position != pos) {
                position = pos;
                OnUpdate();
            }
        }

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
    };

} // namespace elda::models