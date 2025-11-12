#pragma once
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdio>

#include "models/channels_group.h"

namespace elda::models {
    struct Channel;
}

// ===== App constants =====
static constexpr int   CHANNELS        = 64;         // simulate 64 channels
static constexpr float SAMPLE_RATE_HZ  = 1000.0f;    // 1000 Hz
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
    double timeSeconds;
};

// ----------------- Ring buffer with absolute timestamps -----------------
struct Ring {
    std::vector<float> tAbs;                         // absolute seconds
    std::vector<std::vector<float>> data;            // [ch][i]
    int write = 0;
    bool filled = false;
    double now = 0.0;                                // absolute time of next sample

    Ring() {
        tAbs.resize(BUFFER_SIZE, 0.0f);
        data.resize(CHANNELS);
        for (int c=0;c<CHANNELS;++c) data[c].resize(BUFFER_SIZE, 0.0f);
    }

    void reset() {
        write = 0; filled = false; now = 0.0;
        std::fill(tAbs.begin(), tAbs.end(), 0.0f);
        for (int c=0;c<CHANNELS;++c) std::fill(data[c].begin(), data[c].end(), 0.0f);
    }

    inline void push(const std::vector<float>& s) {
        for (int c=0;c<CHANNELS;++c) data[c][write] = s[c];
        tAbs[write] = float(now);
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
    bool isMonitoring       = false;   // START (F5) enables live viewing + sweeper
    bool isRecordingToFile  = false;   // RECORD (F7) toggles writing
    bool isPaused           = false;   // PAUSE (F8) pauses file writing; screen continues
    bool showSettingsPopup  = false;   // Settings popup toggle
    RecordingState recordingState = RecordingState::None; // NEW unified recording state

    // Display choices
    int   winIdx    = 2;               // default 10s
    int   ampIdx    = 4;               // default 200 µV
    int   lastWinIdx = 2;              // track last window index for smooth transitions

    // Derived getters
    float windowSec() const { return WINDOW_OPTIONS[winIdx]; }
    int   ampPPuV()   const { return AMP_PP_UV_OPTIONS[ampIdx]; }
    float gainMul()   const { return float(AMP_PP_UV_OPTIONS[ampIdx]) / AMP_REF_PP_UV; }

    // Runtime noise/artifacts controls (0..5x)
    float noiseScale    = 1.0f;
    float artifactScale = 1.0f;

    // Signal + time
    Ring         ring;
    SampleClock  sampler{SAMPLE_RATE_HZ};

    // ===== Display clock driven by a playhead (freezes when NOT monitoring) =====
    std::chrono::steady_clock::time_point lastTick  = std::chrono::steady_clock::now();
    double playheadSeconds     = 0.0;  // only advances if monitoring
    std::vector<PauseMark> pauseMarks;

    // ===== NEW: Channel configuration (for state manager) =====
    std::string currentChannelGroupName = "Default";
    std::vector<const elda::models::Channel*> selectedChannels;
    const std::vector<elda::models::Channel>* availableChannels;

    std::vector<elda::models::ChannelsGroup> availableGroups;
    // ===== NEW: Recording timing (for state manager) =====
    double recordingStartTime = 0.0;

    // ===== Channel names for display =====
    std::vector<std::string> chNames;

    // Constructor
    AppState();

    // Initialize available channels
    void InitializeChannels();

    // Initialize available channel groups (loads from service, creates defaults if needed)
    void InitializeGroupChannels();

    // Create default channel groups if less than 3 exist (NEW!)
    void CreateDefaultGroups();

    // Get current EEG time
    double currentEEGTime() const { return playheadSeconds; }

    // Tick the display clock (only if monitoring)
    void tickDisplay(bool isMonitoring) {
        auto now = std::chrono::steady_clock::now();
        if (isMonitoring) {
            double dt = std::chrono::duration<double>(now - lastTick).count();
            playheadSeconds += dt;
        }
        lastTick = now;
    }
};

// ----------------- Synthetic EEG generator -----------------
struct SynthEEG {
    std::mt19937 rng{std::random_device{}()};
    std::normal_distribution<float> normDist{0.0f, 1.0f};
    std::uniform_real_distribution<float> uni01{0.0f, 1.0f};

    // Per-channel baseline noise
    std::vector<float> blNoise;

    // Oscillators
    float phAlpha[CHANNELS], phBeta[CHANNELS], phTheta[CHANNELS];
    float drift = 0.0f;
    float driftTarget = 0.0f;
    float driftTimer = 0.0f;

    // Eye blink
    float blinkPhase = 0.0f;
    float blinkActive = 0.0f;

    // Burst
    float burstPhase = 0.0f;
    float burstAmplitude = 0.0f;
    float burstTimer = 0.0f;

    // Spike
    float spikeTimer = 0.0f;
    float spikeAmp = 0.0f;

    SynthEEG() {
        blNoise.resize(CHANNELS, 0.0f);
        for(int c=0; c<CHANNELS; ++c){
            phAlpha[c] = uni01(rng) * 6.283f;
            phBeta[c]  = uni01(rng) * 6.283f;
            phTheta[c] = uni01(rng) * 6.283f;
        }
    }

    void next(std::vector<float>& out) {
        if(out.size()!=(size_t)CHANNELS) out.resize(CHANNELS);

        // Global drift
        driftTimer -= 1.0f / SAMPLE_RATE_HZ;
        if(driftTimer <= 0.0f){
            driftTarget = (uni01(rng)-0.5f)*4.0f;
            driftTimer = 2.0f + uni01(rng)*3.0f;
        }
        drift += 0.02f*(driftTarget - drift);

        // Eye blink
        if(uni01(rng) < 0.0001f){
            blinkActive = 1.0f;
            blinkPhase  = 0.0f;
        }
        if(blinkActive > 0.0f){
            blinkPhase += (2.0f*3.14159f)/(0.15f*SAMPLE_RATE_HZ);
            if(blinkPhase >= 3.14159f){
                blinkActive = 0.0f; blinkPhase = 0.0f;
            }
        }
        float blink = (blinkActive > 0.0f) ? 40.0f*std::sin(blinkPhase) : 0.0f;

        // Burst
        burstTimer -= 1.0f/SAMPLE_RATE_HZ;
        if(burstTimer<=0.0f && uni01(rng)<0.002f){
            burstAmplitude = 20.0f + uni01(rng)*30.0f;
            burstTimer = 0.3f + uni01(rng)*0.5f;
            burstPhase = 0.0f;
        }
        float burst = 0.0f;
        if(burstTimer>0.0f){
            burstPhase += (2.0f*3.14159f*15.0f)/SAMPLE_RATE_HZ;
            burst = burstAmplitude * std::sin(burstPhase)*std::exp(-5.0f*(1.0f-(burstTimer/(0.3f+0.5f))));
        }

        // Spike
        spikeTimer -= 1.0f/SAMPLE_RATE_HZ;
        if(spikeTimer<=0.0f && uni01(rng)<0.0005f){
            spikeAmp = 50.0f + uni01(rng)*50.0f;
            spikeTimer = 0.05f;
        }
        float spike = 0.0f;
        if(spikeTimer>0.0f){
            float t = 1.0f - (spikeTimer/0.05f);
            spike = spikeAmp*(t<0.5f ? t*2.0f : 2.0f*(1.0f-t));
        }

        // 50 Hz mains
        static float mainsPhase=0.0f;
        mainsPhase += (2.0f*3.14159f*50.0f)/SAMPLE_RATE_HZ;
        if(mainsPhase>6.28f) mainsPhase-=6.28f;
        float mains = 0.5f*std::sin(mainsPhase);

        // Per-channel
        for(int c=0; c<CHANNELS; ++c){
            float alpha = 8.0f *std::sin(phAlpha[c]);
            float beta  = 4.0f *std::sin(phBeta[c]);
            float theta = 6.0f *std::sin(phTheta[c]);

            phAlpha[c] += (2.0f*3.14159f*10.0f)/SAMPLE_RATE_HZ;
            phBeta[c]  += (2.0f*3.14159f*20.0f)/SAMPLE_RATE_HZ;
            phTheta[c] += (2.0f*3.14159f* 5.0f)/SAMPLE_RATE_HZ;

            blNoise[c] += 0.02f*(normDist(rng)-blNoise[c]);

            float blinkMul = (c<8) ? 1.5f : 0.3f;
            float y = drift + alpha + beta + theta + blNoise[c] + mains
                      + blinkMul*blink + burst + spike;

            if(uni01(rng) < 2e-6f){
                y += (uni01(rng)-0.5f)*8.0f;
            }

            out[c] = y;
        }
    }
};

// Small helpers for header +/- buttons
inline void decIdx(int& idx, int /*count*/) { if (idx > 0) --idx; }
inline void incIdx(int& idx, int count)     { if (idx < count-1) ++idx; }