#pragma once
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

static constexpr int   CHANNELS        = 64;         // simulate 64
static constexpr float SAMPLE_RATE_HZ  = 1000.0f;    // 1000 Hz
static constexpr int   BUFFER_SECONDS  = 20;
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
    double now = 0.0;

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

// ----------------- Precise sample scheduler -----------------
struct SampleClock {
    using clock = std::chrono::high_resolution_clock;
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

// ----------------- Shared app state -----------------
struct AppState {
    // acquisition
    bool  recording = true;
    bool  stopped   = false;

    // choices
    int   winIdx    = 2; // default 10s
    int   ampIdx    = 4; // default 200 µV

    // derived
    float windowSec() const { return WINDOW_OPTIONS[winIdx]; }
    int   ampPPuV()   const { return AMP_PP_UV_OPTIONS[ampIdx]; }
    float gainMul()   const { return float(AMP_PP_UV_OPTIONS[ampIdx]) / AMP_REF_PP_UV; }

    // signal + time
    Ring         ring;
    SampleClock  sampler{SAMPLE_RATE_HZ};

    // channel names
    std::vector<std::string> chNames;

    AppState() {
        chNames.reserve(CHANNELS);
        for (int i=0;i<CHANNELS;++i){ char b[16]; std::snprintf(b,sizeof(b),"Ch%02d",i+1); chNames.emplace_back(b); }
    }
};

// ----------------- Synthetic EEG generator -----------------
struct SynthEEG {
    std::mt19937 rng{std::random_device{}()};
    std::normal_distribution<float> white{0.f, 0.7f};
    std::vector<float> phaseA{std::vector<float>(CHANNELS,0.f)};
    std::vector<float> phaseB{std::vector<float>(CHANNELS,0.f)};
    std::vector<float> phaseD{std::vector<float>(CHANNELS,0.f)};

    void next(std::vector<float>& out) {
        out.resize(CHANNELS);
        for (int c=0;c<CHANNELS;++c){
            float drift = 3.0f * std::sin(phaseD[c]);            phaseD[c] += 0.03f + 0.00007f*float(c);
            float alpha = (2.0f + 0.3f*(c%5))*std::sin(phaseA[c]); phaseA[c] += 2.0f*float(M_PI)*10.0f / SAMPLE_RATE_HZ;
            float beta  = (1.2f + 0.2f*(c%3))*std::sin(phaseB[c]);  phaseB[c] += 2.0f*float(M_PI)*18.0f / SAMPLE_RATE_HZ;
            float spike = (std::uniform_real_distribution<float>(0.f,1.f)(rng) > 0.999f)
                            ? (8.f * ((rng()&1)?1.f:-1.f)) : 0.f;
            out[c] = drift + alpha + beta + white(rng) + spike;   // arbitrary units; scaled at draw time
        }
    }
};

// Small helpers for header +/- buttons
inline void decIdx(int& idx, int /*count*/) { if (idx > 0) --idx; }
inline void incIdx(int& idx, int count)     { if (idx < count-1) ++idx; }
