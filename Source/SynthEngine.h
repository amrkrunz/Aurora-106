#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace aurora
{
// DSP deliberately has no JUCE dependency and is testable without a DAW.
struct Settings
{
    float lfoRate = 0.25f, lfoDelay = 0.0f;
    int range = 1;                        // 0=16', 1=8', 2=4'
    float dcoLfo = 0.0f, pwm = 0.50f;
    bool pwmManual = false, pulse = true, saw = true;
    float sub = 0.30f, noise = 0.0f;
    int hpf = 1;
    float cutoff = 0.75f, resonance = 0.08f, vcfEnv = 0.0f;
    bool envInverse = false;
    float vcfLfo = 0.0f, keyFollow = 0.50f;
    bool vcaGate = false;
    float vcaLevel = 0.70f;
    float attack = 0.02f, decay = 0.34f, sustain = 0.75f, release = 0.25f;
    int chorus = 1;
    float portamento = 0.0f, drift = 0.10f, master = 0.75f;
};

class SynthEngine
{
public:
    static constexpr int numVoices = 6;
    void prepare(double newSampleRate);
    void reset();
    void setSettings(const Settings& newSettings) noexcept;
    void noteOn(int note, float velocity);
    void noteOff(int note);
    void sustainPedal(bool pressed);
    void pitchWheel(int value14);
    // MIDI CC1 (0..127): adds vibrato, independent of the DCO LFO fader.
    void modulationWheel(int value7);
    void allNotesOff();
    void render(float* left, float* right, int numSamples) noexcept;
    int activeVoices() const noexcept;

private:
    struct Envelope
    {
        enum class Stage { idle, attack, decay, sustain, release } stage = Stage::idle;
        double level = 0.0;
        void on() noexcept { stage = Stage::attack; }
        void off() noexcept { if (stage != Stage::idle) stage = Stage::release; }
        double tick(double sampleRate, const Settings& s) noexcept;
    };
    struct Voice
    {
        bool active = false, keyDown = false, deferredOff = false;
        int note = -1;
        uint64_t age = 0;
        double phase = 0.0, subPhase = 0.0, hz = 440.0, targetHz = 440.0;
        double lp[4]{};
        double hpMemory = 0.0, gateLevel = 0.0, dcMemory = 0.0;
        double centsOffset = 0.0;
        Envelope env;
        void start(int key, double target, double initial, uint64_t newAge, double driftCents) noexcept;
        void releaseKey() noexcept;
        double tick(double fs, const Settings& s, double lfo, double bend,
                    double modulation) noexcept;
    };

    struct Chorus
    {
        static constexpr int size = 32768;
        std::array<double, size> left{}, right{};
        int position = 0;
        double phase = 0.0, preL = 0.0, preR = 0.0;
        double wetL = 0.0, wetR = 0.0;
        double processL(double in, double sampleRate, int mode, double noise) noexcept;
        double processR(double in, double sampleRate, int mode, double noise) noexcept;
        void advance(double sampleRate, int mode) noexcept;
        double read(const std::array<double, size>& buffer, double delaySamples) const noexcept;
        void clear() noexcept;
    } chorus;

    static double frequency(int midiNote) noexcept;
    static double triangle(double phase) noexcept;
    static double polyBlep(double t, double delta) noexcept;
    uint32_t nextRandom() noexcept;
    std::array<Voice, numVoices> voices{};
    Settings settings{};
    double sampleRate = 44100.0, lfoPhase = 0.0, lfoElapsed = 0.0;
    double pitchRatio = 1.0, modulationDepth = 0.0, lastHz = 0.0;
    uint64_t ageCounter = 0;
    uint32_t rngState = 0x7106a11u;
    bool pedalDown = false;
};
} // namespace aurora
