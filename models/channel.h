#pragma once
#include <string>

namespace elda::models {

    struct Channel {
        std::string id;              // Unique identifier (e.g., "CH001")
        std::string name;            // Display name (e.g., "Fp1")
        std::string color;           // Hex color for visualization
        bool selected;               // Whether channel is selected for display

        // Hardware properties
        int amplifierChannel;        // Physical channel on amplifier
        std::string signalType;      // EEG, ECG, EMG, etc.
        double sensorGain;           // Gain setting
        double sensorOffset;         // Baseline offset

        // Filtering (if needed in model)
        bool filtered;
        double highPassCutoff;
        double lowPassCutoff;

        Channel(const std::string& id_,
                const std::string& name_,
                const std::string& color_ = "#FFFFFF")
            : id(id_)
            , name(name_)
            , color(color_)
            , selected(false)
            , amplifierChannel(-1)
            , signalType("EEG")
            , sensorGain(1.0)
            , sensorOffset(0.0)
            , filtered(false)
            , highPassCutoff(0.0)
            , lowPassCutoff(0.0) {}
    };

} // namespace elda