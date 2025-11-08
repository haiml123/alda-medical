#pragma once
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdio>


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
    inline int size() const { return filled ? BUFFER_SIZE : write; }
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
    bool isRecordingToFile  = false;   // RECORD (F7) toggles writing (not implemented yet)
    bool isPaused           = false;   // PAUSE (F8) pauses file writing; screen continues
    bool showSettingsPopup  = false;   // Settings popup toggle

    // choices
    int   winIdx    = 2; // default 10s
    int   ampIdx    = 4; // default 200 µV
    int   lastWinIdx = 2; // ADD THIS - track last window index for smooth transitions


    // derived
    float windowSec() const { return WINDOW_OPTIONS[winIdx]; }
    int   ampPPuV()   const { return AMP_PP_UV_OPTIONS[ampIdx]; }
    float gainMul()   const { return float(AMP_PP_UV_OPTIONS[ampIdx]) / AMP_REF_PP_UV; }

    // runtime noise/artifacts controls (0..5x)
    float noiseScale    = 1.0f;
    float artifactScale = 1.0f;

    // signal + time
    Ring         ring;
    SampleClock  sampler{SAMPLE_RATE_HZ};

    // ===== display clock driven by a playhead (freezes when NOT monitoring) =====
    std::chrono::steady_clock::time_point lastTick  = std::chrono::steady_clock::now();
    double playheadSeconds     = 0.0;  // only advances while isMonitoring
    double displayNow          = 0.0;  // raw (for reference)
    double displayNowSmoothed  = 0.0;  // EMA smoothed (used by sweeper)
    bool   emaPrimed           = false;

    // ========================================================================
    // FIXED: Frame-rate independent exponential smoothing
    // ========================================================================
    // called once per frame
    inline void tickDisplay(bool advance_playhead) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - lastTick).count();
        lastTick = now;

        if (advance_playhead) {
            playheadSeconds += dt;
        }

        const double lambda = 8.0; // Tunable smoothing parameter (recommended: 5-10)

        if (!emaPrimed) {
            displayNowSmoothed = playheadSeconds;
            emaPrimed = true;
        }
        else {
            // Frame-rate independent exponential decay formula
            displayNowSmoothed = playheadSeconds +
                                 (displayNowSmoothed - playheadSeconds) * std::exp(-lambda * dt);
        }

        displayNow = playheadSeconds;
    }

    // channel names
    std::vector<std::string> chNames;

    // ===== Channel configuration =====
    std::vector<elda::models::Channel> availableChannels;

    // pause marks (timestamped in EEG/playhead seconds)
    struct PauseMark { double t_start; };
    std::vector<PauseMark> pauseMarks;

    AppState();

    // Helper method to initialize channels
    void InitializeChannels();

    // helper for marks / logging
    double currentEEGTime() const { return playheadSeconds; }
};

// ----------------- Synthetic EEG generator (noisier & more random) -----------------
struct SynthEEG {
    // Tunables (base levels; scaled live via noiseScale/artifactScale)
    float mainsHz         = 60.0f;     // set 50.0f if you prefer EU mains
    float mainsAmp        = 1.6f;      // base mains amplitude
    float noiseStdBase    = 2.5f;      // base gaussian noise std
    float blNoiseAlpha    = 0.90f;     // 1-pole LPF for band-limited noise (lower = grainier)
    float dcWanderHz      = 0.08f;     // slow baseline wander
    float blinkChance     = 4.0e-4f;   // chance per sample per channel to start a blink
    float blinkDurSec     = 0.18f;     // blink duration
    float blinkAmp        = 12.0f;     // blink amplitude
    float burstChance     = 8.0e-5f;   // brief high-freq burst
    float burstDurSec     = 0.12f;
    float burstAmp        = 8.0f;
    float spikeChance     = 1.5e-5f;   // random single-sample spike
    float spikeAmp        = 18.0f;

    // runtime
    std::mt19937 rng{std::random_device{}()};
    std::normal_distribution<float> gauss{0.0f, 1.0f};
    std::uniform_real_distribution<float> uni01{0.0f, 1.0f};

    std::vector<float>   phAlpha, phBeta, phTheta, phDC, phMains;
    std::vector<float>   alphaHz, betaHz, thetaHz;
    std::vector<float>   alphaAmp, betaAmp, thetaAmp, dcAmp, mainsChAmp;
    std::vector<float>   blNoise;
    std::vector<int>     blinkLeft, burstLeft;

    SynthEEG() {
        phAlpha.resize(CHANNELS); phBeta.resize(CHANNELS); phTheta.resize(CHANNELS);
        phDC.resize(CHANNELS); phMains.resize(CHANNELS);
        alphaHz.resize(CHANNELS); betaHz.resize(CHANNELS); thetaHz.resize(CHANNELS);
        alphaAmp.resize(CHANNELS); betaAmp.resize(CHANNELS); thetaAmp.resize(CHANNELS);
        dcAmp.resize(CHANNELS); mainsChAmp.resize(CHANNELS);
        blNoise.resize(CHANNELS, 0.0f);
        blinkLeft.resize(CHANNELS, 0);
        burstLeft.resize(CHANNELS, 0);

        for (int c=0;c<CHANNELS;++c) {
            phAlpha[c] = uni01(rng) * 2.f * float(M_PI);
            phBeta [c] = uni01(rng) * 2.f * float(M_PI);
            phTheta[c] = uni01(rng) * 2.f * float(M_PI);
            phDC   [c] = uni01(rng) * 2.f * float(M_PI);
            phMains[c] = uni01(rng) * 2.f * float(M_PI);

            alphaHz[c] = 8.5f  + 3.5f * uni01(rng);
            betaHz [c] = 15.f  + 10.f * uni01(rng);
            thetaHz[c] = 4.0f  + 3.0f * uni01(rng);

            alphaAmp[c] = 2.0f + 1.5f * uni01(rng);
            betaAmp [c] = 1.0f + 1.0f * uni01(rng);
            thetaAmp[c] = 0.7f + 0.8f * uni01(rng);
            dcAmp   [c] = 2.5f + 2.0f * uni01(rng);

            mainsChAmp[c] = mainsAmp * (0.6f + 0.8f * uni01(rng));
        }
    }

    void next(std::vector<float>& out) {
        out.resize(CHANNELS);

        float mainsJitter = (uni01(rng) - 0.5f) * 0.6f; // ±0.3 Hz

        for (int c=0;c<CHANNELS;++c) {
            float aHz = alphaHz[c] + 0.15f * std::sin(0.03f * phAlpha[c]);
            float bHz = betaHz [c] + 0.25f * std::sin(0.02f * phBeta [c]);
            float tHz = thetaHz[c] + 0.10f * std::sin(0.025f* phTheta[c]);

            phAlpha[c] += 2.f * float(M_PI) * aHz / SAMPLE_RATE_HZ;
            phBeta [c] += 2.f * float(M_PI) * bHz / SAMPLE_RATE_HZ;
            phTheta[c] += 2.f * float(M_PI) * tHz / SAMPLE_RATE_HZ;
            phDC   [c] += 2.f * float(M_PI) * dcWanderHz / SAMPLE_RATE_HZ;
            phMains[c] += 2.f * float(M_PI) * (mainsHz + mainsJitter) / SAMPLE_RATE_HZ;

            float alpha = alphaAmp[c] * std::sin(phAlpha[c]);
            float beta  = betaAmp [c] * std::sin(phBeta [c]);
            float theta = thetaAmp[c] * std::sin(phTheta[c]);

            float drift = dcAmp[c] * std::sin(phDC[c]);

            float wn = noiseStdBase * gauss(rng);
            blNoise[c] = blNoiseAlpha * blNoise[c] + (1.f - blNoiseAlpha) * wn;

            float mains = mainsChAmp[c] * std::sin(phMains[c]);

            if (blinkLeft[c] <= 0 && ((c < 8) ? uni01(rng) < 3.f*blinkChance
                                              : uni01(rng) < blinkChance)) {
                blinkLeft[c] = int(std::round(blinkDurSec * SAMPLE_RATE_HZ));
            }
            float blink = 0.f;
            if (blinkLeft[c] > 0) {
                float k = 1.f - std::cos(float(M_PI) * (1.f - float(blinkLeft[c]) / (blinkDurSec * SAMPLE_RATE_HZ)));
                blink = blinkAmp * k * 0.5f;
                --blinkLeft[c];
            }

            if (burstLeft[c] <= 0 && uni01(rng) < burstChance) {
                burstLeft[c] = int(std::round(burstDurSec * SAMPLE_RATE_HZ));
            }
            float burst = 0.f;
            if (burstLeft[c] > 0) {
                float hf = std::sin(phBeta[c]*2.5f) + 0.6f*std::sin(phAlpha[c]*3.3f);
                burst = burstAmp * (0.5f*std::abs(hf) + 0.5f*std::abs(gauss(rng)));
                --burstLeft[c];
            }

            float spike = 0.f;
            if (uni01(rng) < spikeChance) {
                if ((rng() & 127) == 0) spike = ((rng() & 1) ? spikeAmp : -spikeAmp);
            }

            float y = drift + alpha + beta + theta + blNoise[c] + mains + blink + burst + spike;

            if (uni01(rng) < 2e-6f) {
                y += (uni01(rng) - 0.5f) * 8.0f;
            }

            out[c] = y;
        }
    }
};

// Small helpers for header +/- buttons
inline void decIdx(int& idx, int /*count*/) { if (idx > 0) --idx; }
inline void incIdx(int& idx, int count)     { if (idx < count-1) ++idx; }