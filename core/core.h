#pragma once
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdio>

#include "models/channels_group.h"
#include "models/session.h"

namespace elda::models {
    struct Channel;
}

// ===== App constants =====
static constexpr int   CHANNELS        = 64;         // simulate 64 channels
static constexpr float SAMPLE_RATE_HZ  = 20.0f;    // 1000 Hz
static constexpr int   BUFFER_SECONDS  = 25;
static constexpr int   BUFFER_SIZE     = int(SAMPLE_RATE_HZ * BUFFER_SECONDS);

// X-window choices (sec)
static const float WINDOW_OPTIONS[] = {1.f, 5.f, 10.f, 15.f, 20.f};
static constexpr int WINDOW_COUNT = sizeof(WINDOW_OPTIONS)/sizeof(WINDOW_OPTIONS[0]);

// Amplitude (peak-to-peak) choices (µV)
static const int AMP_PP_UV_OPTIONS[] = {10, 20, 50, 100, 200, 500, 1000};
static constexpr int AMP_COUNT = sizeof(AMP_PP_UV_OPTIONS)/sizeof(AMP_PP_UV_OPTIONS[0]);
static constexpr float AMP_REF_PP_UV = 100.0f;       // 100 µV pp => gain 1.0

enum class RecordingState {
    None = 0,
    Recording,
    Paused
};
// Pause mark structure
struct PauseMark {
    double time_seconds;
};

// ----------------- Ring buffer with absolute timestamps -----------------
struct Ring {
    std::vector<float> t_abs;                         // absolute seconds
    std::vector<std::vector<float>> data;            // [ch][i]
    int write = 0;
    bool filled = false;
    double now = 0.0;                                // absolute time of next sample

    Ring() {
        t_abs.resize(BUFFER_SIZE, 0.0f);
        data.resize(CHANNELS);
        for (int c=0;c<CHANNELS;++c) data[c].resize(BUFFER_SIZE, 0.0f);
    }

    void reset() {
        write = 0; filled = false; now = 0.0;
        std::fill(t_abs.begin(), t_abs.end(), 0.0f);
        for (int c=0;c<CHANNELS;++c) std::fill(data[c].begin(), data[c].end(), 0.0f);
    }

    inline void push(const std::vector<float>& s) {
        for (int c=0;c<CHANNELS;++c) data[c][write] = s[c];
        t_abs[write] = float(now);
        now += 1.0 / SAMPLE_RATE_HZ;
        write = (write + 1) % BUFFER_SIZE;
        if (write == 0) filled = true;
    }

    inline int size() const {
        return filled ? BUFFER_SIZE : write;
    }
};

// ----------------- Precise sample scheduler (holds 1 kHz average) -----------------
struct SampleClock {
    using clock = std::chrono::steady_clock;
    clock::time_point next;
    double dt;
    explicit SampleClock(double sr) : next(clock::now()), dt(1.0 / sr) {}

    int due() {
        auto n = clock::now();
        int k = 0;
        while (std::chrono::duration<double>(n - next).count() >= 0.0) {
            next += std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(dt));
            ++k;
        }
        return k;
    }
};

// ----------------- Shared app state (NeoRec-style) -----------------
struct AppState {
    // ===== NeoRec control state =====
    bool is_monitoring       = false;   // START (F5) enables live viewing + sweeper
    bool is_recording_to_file  = false;   // RECORD (F7) toggles writing
    bool is_paused           = false;   // PAUSE (F8) pauses file writing; screen continues
    bool show_settings_popup  = false;   // Settings popup toggle
    RecordingState recording_state = RecordingState::None; // NEW unified recording state
    std::optional<elda::models::Session> current_session;

    // Display choices
    int   win_idx    = 2;               // default 10s
    int   amp_idx    = 4;               // default 200 µV
    int   last_win_idx = 2;              // track last window index for smooth transitions

    // Derived getters
    float window_sec() const { return WINDOW_OPTIONS[win_idx]; }
    int   amp_pp_uv()   const { return AMP_PP_UV_OPTIONS[amp_idx]; }
    float gain_mul()   const { return float(AMP_PP_UV_OPTIONS[amp_idx]) / AMP_REF_PP_UV; }

    // Runtime noise/artifacts controls (0..5x)
    float noise_scale    = 1.0f;
    float artifact_scale = 1.0f;

    // Signal + time
    Ring         ring;
    SampleClock  sampler{SAMPLE_RATE_HZ};

    // ===== Display clock driven by a playhead (freezes when NOT monitoring) =====
    std::chrono::steady_clock::time_point last_tick  = std::chrono::steady_clock::now();
    double playhead_seconds     = 0.0;  // only advances if monitoring
    std::vector<PauseMark> pause_marks;

    // ===== NEW: Channel configuration (for state manager) =====
    std::string current_channel_group_name = "Default";
    std::vector<const elda::models::Channel*> selected_channels;
    const std::vector<elda::models::Channel>* available_channels;

    std::vector<elda::models::ChannelsGroup> available_groups;
    // ===== NEW: Recording timing (for state manager) =====
    double recording_start_time = 0.0;

    // ===== Channel names for display =====
    std::vector<std::string> ch_names;

    // Constructor
    AppState();

    // Initialize available channels
    void initialize_channels();

    // Initialize available channel groups (loads from service, creates defaults if needed)
    void initialize_group_channels();

    // Create default channel groups if less than 3 exist (NEW!)
    void create_default_groups();

    // Get current EEG time
    double current_eeg_time() const { return playhead_seconds; }

    // Tick the display clock (only if monitoring)
    void tick_display(bool is_monitoring) {
        auto now = std::chrono::steady_clock::now();
        if (is_monitoring) {
            double dt = std::chrono::duration<double>(now - last_tick).count();
            playhead_seconds += dt;
        }
        last_tick = now;
    }
};

// ----------------- Synthetic EEG generator -----------------
struct SynthEEG {
    std::mt19937 rng{std::random_device{}()};
    std::normal_distribution<float> norm_dist{0.0f, 1.0f};
    std::uniform_real_distribution<float> uni_01{0.0f, 1.0f};

    // Per-channel baseline noise
    std::vector<float> bl_noise;

    // Oscillators
    float ph_alpha[CHANNELS], ph_beta[CHANNELS], ph_theta[CHANNELS];
    float drift = 0.0f;
    float drift_target = 0.0f;
    float drift_timer = 0.0f;

    // Eye blink
    float blink_phase = 0.0f;
    float blink_active = 0.0f;

    // Burst
    float burst_phase = 0.0f;
    float burst_amplitude = 0.0f;
    float burst_timer = 0.0f;

    // Spike
    float spike_timer = 0.0f;
    float spike_amp = 0.0f;

    SynthEEG() {
        bl_noise.resize(CHANNELS, 0.0f);
        for(int c=0; c<CHANNELS; ++c){
            ph_alpha[c] = uni_01(rng) * 6.283f;
            ph_beta[c]  = uni_01(rng) * 6.283f;
            ph_theta[c] = uni_01(rng) * 6.283f;
        }
    }

    void next(std::vector<float>& out) {
        if(out.size()!=(size_t)CHANNELS) out.resize(CHANNELS);

        // Global drift
        drift_timer -= 1.0f / SAMPLE_RATE_HZ;
        if(drift_timer <= 0.0f){
            drift_target = (uni_01(rng)-0.5f)*4.0f;
            drift_timer = 2.0f + uni_01(rng)*3.0f;
        }
        drift += 0.02f*(drift_target - drift);

        // Eye blink
        if(uni_01(rng) < 0.0001f){
            blink_active = 1.0f;
            blink_phase  = 0.0f;
        }
        if(blink_active > 0.0f){
            blink_phase += (2.0f*3.14159f)/(0.15f*SAMPLE_RATE_HZ);
            if(blink_phase >= 3.14159f){
                blink_active = 0.0f; blink_phase = 0.0f;
            }
        }
        float blink = (blink_active > 0.0f) ? 40.0f*std::sin(blink_phase) : 0.0f;

        // Burst
        burst_timer -= 1.0f/SAMPLE_RATE_HZ;
        if(burst_timer<=0.0f && uni_01(rng)<0.002f){
            burst_amplitude = 20.0f + uni_01(rng)*30.0f;
            burst_timer = 0.3f + uni_01(rng)*0.5f;
            burst_phase = 0.0f;
        }
        float burst = 0.0f;
        if(burst_timer>0.0f){
            burst_phase += (2.0f*3.14159f*15.0f)/SAMPLE_RATE_HZ;
            burst = burst_amplitude * std::sin(burst_phase)*std::exp(-5.0f*(1.0f-(burst_timer/(0.3f+0.5f))));
        }

        // Spike
        spike_timer -= 1.0f/SAMPLE_RATE_HZ;
        if(spike_timer<=0.0f && uni_01(rng)<0.0005f){
            spike_amp = 50.0f + uni_01(rng)*50.0f;
            spike_timer = 0.05f;
        }
        float spike = 0.0f;
        if(spike_timer>0.0f){
            float t = 1.0f - (spike_timer/0.05f);
            spike = spike_amp*(t<0.5f ? t*2.0f : 2.0f*(1.0f-t));
        }

        // 50 Hz mains
        static float mains_phase=0.0f;
        mains_phase += (2.0f*3.14159f*50.0f)/SAMPLE_RATE_HZ;
        if(mains_phase>6.28f) mains_phase-=6.28f;
        float mains = 0.5f*std::sin(mains_phase);

        // Per-channel
        for(int c=0; c<CHANNELS; ++c){
            float alpha = 8.0f *std::sin(ph_alpha[c]);
            float beta  = 4.0f *std::sin(ph_beta[c]);
            float theta = 6.0f *std::sin(ph_theta[c]);

            ph_alpha[c] += (2.0f*3.14159f*10.0f)/SAMPLE_RATE_HZ;
            ph_beta[c]  += (2.0f*3.14159f*20.0f)/SAMPLE_RATE_HZ;
            ph_theta[c] += (2.0f*3.14159f* 5.0f)/SAMPLE_RATE_HZ;

            bl_noise[c] += 0.02f*(norm_dist(rng)-bl_noise[c]);

            float blink_mul = (c<8) ? 1.5f : 0.3f;
            float y = drift + alpha + beta + theta + bl_noise[c] + mains
                      + blink_mul*blink + burst + spike;

            if(uni_01(rng) < 2e-6f){
                y += (uni_01(rng)-0.5f)*8.0f;
            }

            out[c] = y;
        }
    }
};

// Small helpers for header +/- buttons
inline void dec_idx(int& idx, int /*count*/) { if (idx > 0) --idx; }
inline void inc_idx(int& idx, int count)     { if (idx < count-1) ++idx; }
