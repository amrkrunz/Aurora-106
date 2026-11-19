#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"
#include <array>
#include <atomic>
#include <vector>

namespace aurora
{
// Values in patches are actual APVTS parameter values, not 0..1 encodings of choices.
enum Param : size_t
{
    lfoRate, lfoDelay, range, dcoLfo, pwm, pwmManual, pulse, saw,
    sub, noise, hpf, cutoff, resonance, envAmount, envInverse,
    filterLfo, keyTrack, vcaGate, vcaLevel, attack, decay, sustain,
    release, chorus, portamento, drift, master, paramCount
};

extern const std::array<const char*, paramCount> parameterIds;
struct Patch
{
    juce::String name;
    std::array<float, paramCount> values{};
};

class Processor final : public juce::AudioProcessor
{
public:
    Processor();
    ~Processor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return static_cast<int>(patches.size()); }
    int getCurrentProgram() override { return selectedProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& state() noexcept { return parameters; }
    juce::MidiKeyboardState keyboardState;

    // Performance wheels are real-time controls, not preset parameters.
    // Atomic handoff keeps the GUI thread from touching the DSP voice engine.
    void setPitchBendFromUI(int value14) noexcept;
    void setModulationFromUI(int value7) noexcept;
    int pitchBendPosition() const noexcept { return pitchPosition.load(std::memory_order_relaxed); }
    int modulationPosition() const noexcept { return modPosition.load(std::memory_order_relaxed); }

    int importSysEx(const juce::File&, juce::String& error);
    bool exportPreset(const juce::File&, juce::String& error);
    bool importPreset(const juce::File&, juce::String& error);
    void refreshCustomPatchName();
    int programCount() const noexcept { return static_cast<int>(patches.size()); }
    juce::String programName(int index) const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    static std::vector<Patch> makePresets();
    float value(Param id) const noexcept;
    std::array<float, paramCount> currentValues() const noexcept;
    void loadValues(const std::array<float, paramCount>& values);
    Settings readSettings() const noexcept;

    juce::AudioProcessorValueTreeState parameters;
    SynthEngine engine;
    std::vector<Patch> patches;
    int selectedProgram = 0;
    std::atomic<int> pitchPosition { 8192 }; // MIDI 14-bit centre
    std::atomic<int> modPosition { 0 };      // CC1 (0..127)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
} // namespace aurora
