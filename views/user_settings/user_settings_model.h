#pragma once

#include <string>

namespace elda::views::user_settings
{

enum class AcquisitionMode
{
    NORMAL = 0,
    MRI = 1,
    CUSTOM = 2
};

inline const char* acquisition_mode_to_string(AcquisitionMode mode)
{
    switch (mode)
    {
        case AcquisitionMode::NORMAL:
            return "Normal";
        case AcquisitionMode::MRI:
            return "MRI";
        case AcquisitionMode::CUSTOM:
            return "Custom";
        default:
            return "Unknown";
    }
}

// Clean data structure - no char buffers, just std::string
struct PatientInfo
{
    std::string technician;
    std::string subject_code;
    std::string age;  // Text input for age
    std::string notes;
};

// MRI-specific settings
struct MRISettings
{
    std::string scanner_id;
    int tr_ms = 2000;
    bool gradient_correction = false;
};

// Custom/NVX acquisition settings
struct CustomSettings
{
    int nvx_mode = 0;                // 0=Normal, 1=Active Shield, 2=Impedance, 3=Test Signal
    int sampling_rate = 1000;        // 250, 500, 1000, 5000, 10000, 25000
    int nvx_gain = 0;                // 0, 1
    int nvx_power_save = 1;          // 0=Disable, 1=Enable
    int nvx_scan_freq = 0;           // 0=30Hz, 1=80Hz
    int base_adc_sync = 0;           // 0=Internal, 1=External, 2=Manual
    int sw_impedance_reduction = 1;  // 0=Off, 1=On
};

class UserSettingsModel
{
  public:
    UserSettingsModel() = default;

    // Mode
    AcquisitionMode get_mode() const
    {
        return mode_;
    }
    void set_mode(AcquisitionMode mode)
    {
        mode_ = mode;
    }

    // Data accessors
    PatientInfo& patient()
    {
        return patient_;
    }
    const PatientInfo& patient() const
    {
        return patient_;
    }

    MRISettings& mri_settings()
    {
        return mri_settings_;
    }
    const MRISettings& mri_settings() const
    {
        return mri_settings_;
    }

    CustomSettings& custom_settings()
    {
        return custom_settings_;
    }
    const CustomSettings& custom_settings() const
    {
        return custom_settings_;
    }

    // Reset
    void clear()
    {
        mode_ = AcquisitionMode::NORMAL;
        patient_ = {};
        mri_settings_ = {};
        custom_settings_ = {};
    }

  private:
    AcquisitionMode mode_ = AcquisitionMode::NORMAL;
    PatientInfo patient_;
    MRISettings mri_settings_;
    CustomSettings custom_settings_;
};

}  // namespace elda::views::user_settings