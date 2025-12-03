#pragma once

#include <string>
#include <vector>

namespace elda::views::channels_config {

// Signal types per NeoRec documentation
enum class SignalType {
    EEG = 0,    // µV (microvolts)
    ECG_EMG,    // mV (millivolts)
    ICG_IMP,    // Ohm
    SCG_MOV,    // G (gravitational)
    FPG_RESP,   // dimensionless
    GSR,        // S (Siemens)
    TEMP,       // °C
    ROT,        // deg/s
    BP,         // mmHg
    FORCE       // daN
};

inline const char* signal_type_to_string(SignalType type) {
    switch (type) {
        case SignalType::EEG:      return "EEG";
        case SignalType::ECG_EMG:  return "ECG/EMG";
        case SignalType::ICG_IMP:  return "ICG/Imp";
        case SignalType::SCG_MOV:  return "SCG/MOV";
        case SignalType::FPG_RESP: return "FPG/Resp";
        case SignalType::GSR:      return "GSR";
        case SignalType::TEMP:     return "Temp";
        case SignalType::ROT:      return "ROT";
        case SignalType::BP:       return "BP";
        case SignalType::FORCE:    return "Force";
        default:                   return "EEG";
    }
}

// Source differential mode
enum class SourceDiff {
    MONO = 0,   // Monopolar (uses amplifier's internal reference)
    GND,        // Ground reference
    REF         // Reference electrode
};

inline const char* source_diff_to_string(SourceDiff diff) {
    switch (diff) {
        case SourceDiff::MONO: return "Mono";
        case SourceDiff::GND:  return "GND";
        case SourceDiff::REF:  return "REF";
        default:               return "Mono";
    }
}

// HPF options (High-pass filter)
enum class HPFOption {
    DC = 0,     // 0 Hz (no filter)
    HPF_0001,   // 0.001 Hz
    HPF_001,    // 0.01 Hz
    HPF_01,     // 0.1 Hz
    HPF_1       // 1 Hz
};

inline const char* hpf_to_string(HPFOption hpf) {
    switch (hpf) {
        case HPFOption::DC:       return "DC";
        case HPFOption::HPF_0001: return "0.001 Hz";
        case HPFOption::HPF_001:  return "0.01 Hz";
        case HPFOption::HPF_01:   return "0.1 Hz";
        case HPFOption::HPF_1:    return "1 Hz";
        default:                  return "DC";
    }
}

// LPF options (Low-pass filter)
enum class LPFOption {
    NONE = 0,   // No filter
    LPF_250,    // 250 Hz
    LPF_500     // 500 Hz
};

inline const char* lpf_to_string(LPFOption lpf) {
    switch (lpf) {
        case LPFOption::NONE:    return "None";
        case LPFOption::LPF_250: return "250 Hz";
        case LPFOption::LPF_500: return "500 Hz";
        default:                 return "None";
    }
}

// ADF options (Notch filter)
enum class ADFOption {
    OFF = 0,    // No notch filter
    ADF_50,     // 50 Hz
    ADF_60      // 60 Hz
};

inline const char* adf_to_string(ADFOption adf) {
    switch (adf) {
        case ADFOption::OFF:    return "Off";
        case ADFOption::ADF_50: return "50 Hz";
        case ADFOption::ADF_60: return "60 Hz";
        default:                return "Off";
    }
}

// Single channel configuration
struct ChannelConfig {
    int id = 0;                           // Channel number (1-64, 1-136, etc.)
    std::string name;                     // Display name (e.g., "Fp1", "Fp2", "O1")
    SignalType signal_type = SignalType::EEG;
    int source_main = 0;                  // Amplifier channel number
    SourceDiff source_diff = SourceDiff::REF;
    float sensor_gain = 1.0f;             // Gain multiplier
    float sensor_offset = 0.0f;           // Baseline offset (V)
    HPFOption hpf = HPFOption::HPF_0001;  // High-pass filter
    LPFOption lpf = LPFOption::LPF_250;   // Low-pass filter
    ADFOption adf = ADFOption::OFF;       // Notch filter
    bool enabled = true;                  // Whether channel is active
    std::string color = "#1ACC94";        // Display color (hex)
};

// Model for managing all channels
class ChannelsConfigModel {
public:
    ChannelsConfigModel() {
        init_default_channels(64);  // Default to 64 channels
    }

    void init_default_channels(int count) {
        channels_.clear();
        channels_.reserve(count);
        
        // Standard 10-20 electrode names (extended)
        static const char* standard_names[] = {
            "Fp1", "Fp2", "F7", "F3", "Fz", "F4", "F8",
            "T3", "C3", "Cz", "C4", "T4",
            "T5", "P3", "Pz", "P4", "T6",
            "O1", "Oz", "O2",
            "AF7", "AF3", "AFz", "AF4", "AF8",
            "F5", "F1", "F2", "F6",
            "FC5", "FC1", "FC2", "FC6",
            "T7", "C5", "C1", "C2", "C6", "T8",
            "CP5", "CP1", "CP2", "CP6",
            "P7", "P5", "P1", "P2", "P6", "P8",
            "PO7", "PO3", "POz", "PO4", "PO8",
            "Fpz", "CPz", "TP7", "TP8", "FT7", "FT8",
            "A1", "A2", "M1", "M2"
        };
        const int num_standard = sizeof(standard_names) / sizeof(standard_names[0]);
        
        for (int i = 0; i < count; ++i) {
            ChannelConfig ch;
            ch.id = i + 1;
            ch.source_main = i + 1;
            
            // Use standard name if available, otherwise generate
            if (i < num_standard) {
                ch.name = standard_names[i];
            } else {
                ch.name = "Ch" + std::to_string(i + 1);
            }
            
            channels_.push_back(ch);
        }
    }

    // Accessors
    std::vector<ChannelConfig>& channels() { return channels_; }
    const std::vector<ChannelConfig>& channels() const { return channels_; }
    
    ChannelConfig* get_channel(int id) {
        for (auto& ch : channels_) {
            if (ch.id == id) return &ch;
        }
        return nullptr;
    }

    // Selection management
    void select_channel(int id, bool selected) {
        if (selected) {
            if (std::find(selected_ids_.begin(), selected_ids_.end(), id) == selected_ids_.end()) {
                selected_ids_.push_back(id);
            }
        } else {
            selected_ids_.erase(
                std::remove(selected_ids_.begin(), selected_ids_.end(), id),
                selected_ids_.end()
            );
        }
    }
    
    void select_all(bool selected) {
        selected_ids_.clear();
        if (selected) {
            for (const auto& ch : channels_) {
                selected_ids_.push_back(ch.id);
            }
        }
    }
    
    bool is_selected(int id) const {
        return std::find(selected_ids_.begin(), selected_ids_.end(), id) != selected_ids_.end();
    }
    
    const std::vector<int>& selected_ids() const { return selected_ids_; }
    
    // Apply settings to selected channels
    void apply_to_selected(const ChannelConfig& settings) {
        for (int id : selected_ids_) {
            if (auto* ch = get_channel(id)) {
                ch->signal_type = settings.signal_type;
                ch->source_diff = settings.source_diff;
                ch->sensor_gain = settings.sensor_gain;
                ch->sensor_offset = settings.sensor_offset;
                ch->hpf = settings.hpf;
                ch->lpf = settings.lpf;
                ch->adf = settings.adf;
            }
        }
    }

private:
    std::vector<ChannelConfig> channels_;
    std::vector<int> selected_ids_;
};

} // namespace elda::views::channels_config
