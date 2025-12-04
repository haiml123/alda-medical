#pragma once

#include <string>

namespace elda::views::admin_settings
{

// NVX Device Configuration (per customer doc)
struct NVXDeviceConfig
{
    int nvx_mode = 0;        // 0=Normal, 1=Active Shield, 2=Impedance, 3=Test Signal
    int nvx_rate = 0;        // 0=10kHz, 1=50kHz, 2=100kHz
    int nvx_decimation = 0;  // 0, 2, 5, 10, 20, 40
    int nvx_gain = 0;        // 0, 1
    int nvx_power_save = 1;  // 0=Disable, 1=Enable (default: Enable)
    int nvx_scan_freq = 0;   // 0=30Hz, 1=80Hz
};

// Output Settings
struct OutputConfig
{
    int output_file_type = 0;        // 0=EDF+, 1=EEG, 2=Raw (default: EDF+)
    int base_adc_sync = 0;           // 0=Internal, 1=External, 2=Manual
    int sw_impedance_reduction = 1;  // 0=Off, 1=On (default: On)
};

// Channel Settings (defaults for all channels)
struct ChannelDefaults
{
    int source = 2;  // 0=Diff, 1=GND, 2=REF (default: REF)
    int hpf = 1;     // 0=DC, 1=0.001Hz, 2=0.01Hz, 3=0.1Hz, 4=1Hz (default: 0.001Hz)
    int lpf = 1;     // 0=None, 1=250Hz, 2=500Hz (default: 250Hz)
    int adf = 0;     // 0=None, 1=50Hz, 2=60Hz (default: None)
};

class AdminSettingsModel
{
  public:
    AdminSettingsModel() = default;

    // Data accessors
    NVXDeviceConfig& device_config()
    {
        return device_config_;
    }
    const NVXDeviceConfig& device_config() const
    {
        return device_config_;
    }

    OutputConfig& output_config()
    {
        return output_config_;
    }
    const OutputConfig& output_config() const
    {
        return output_config_;
    }

    ChannelDefaults& channel_defaults()
    {
        return channel_defaults_;
    }
    const ChannelDefaults& channel_defaults() const
    {
        return channel_defaults_;
    }

    // Tab state
    int active_tab() const
    {
        return active_tab_;
    }
    void set_active_tab(int tab)
    {
        active_tab_ = tab;
    }

    // Reset
    void clear()
    {
        device_config_ = {};
        output_config_ = {};
        channel_defaults_ = {};
        active_tab_ = 0;
    }

  private:
    NVXDeviceConfig device_config_;
    OutputConfig output_config_;
    ChannelDefaults channel_defaults_;
    int active_tab_ = 0;
};

}  // namespace elda::views::admin_settings
