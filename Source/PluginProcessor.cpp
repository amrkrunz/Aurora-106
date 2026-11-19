#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <initializer_list>

namespace aurora
{
const std::array<const char*, paramCount> parameterIds = {{
    "lfoRate", "lfoDelay", "range", "dcoLfo", "pwm", "pwmManual", "pulse", "saw",
    "sub", "noise", "hpf", "cutoff", "resonance", "envAmount", "envInverse",
    "filterLfo", "keyTrack", "vcaGate", "vcaLevel", "attack", "decay", "sustain",
    "release", "chorus", "portamento", "drift", "master"
}};

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout Processor::makeLayout()
{
    APVTS::ParameterLayout result;
    const auto flt = [&](Param idx, const juce::String& name, float def)
    {
        result.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(parameterIds[idx], 1), name,
            juce::NormalisableRange<float>(0.f, 1.f, 0.0001f), def));
    };
    const auto yes = [&](Param idx, const juce::String& name, bool def)
    {
        result.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(parameterIds[idx], 1), name, def));
    };
    const auto menu = [&](Param idx, const juce::String& name,
                          const juce::StringArray& choices, int def)
    {
        result.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(parameterIds[idx], 1), name, choices, def));
    };
    flt(lfoRate, "LFO rate", .25f); flt(lfoDelay, "LFO delay", 0.f);
    menu(range, "DCO range", { "16'", "8'", "4'" }, 1);
    flt(dcoLfo, "DCO LFO modulation", 0.f); flt(pwm, "PWM depth", .5f);
    yes(pwmManual, "PWM manual mode", false);
    yes(pulse, "Pulse wave", true); yes(saw, "Saw wave", true);
    flt(sub, "Sub oscillator", .30f); flt(noise, "Noise level", 0.f);
    menu(hpf, "Highpass position", { "0 BASS", "1 FLAT", "2 HIGH", "3 HIGH" }, 1);
    flt(cutoff, "VCF frequency", .75f); flt(resonance, "VCF resonance", .08f);
    flt(envAmount, "VCF envelope amount", 0.f); yes(envInverse, "VCF envelope invert", false);
    flt(filterLfo, "VCF LFO amount", 0.f); flt(keyTrack, "VCF key follow", .5f);
    yes(vcaGate, "VCA gate mode", false); flt(vcaLevel, "VCA level", .7f);
    flt(attack, "Attack", .02f); flt(decay, "Decay", .34f);
    flt(sustain, "Sustain", .75f); flt(release, "Release", .25f);
    menu(chorus, "Chorus", { "OFF", "I", "II" }, 1);
    flt(portamento, "Portamento", 0.f);
    flt(drift, "Voice drift", .10f); flt(master, "Output level", .75f);
    return result;
}

namespace
{
Patch base(juce::String name)
{
    Patch p;
    p.name = std::move(name);
    p.values = {{ .25f,0.f, 1.f,0.f,.50f,0.f,1.f,1.f,
                  .30f,0.f,1.f,.75f,.08f,0.f,0.f,
                  0.f,.50f,0.f,.70f,.02f,.34f,.75f,
                  .25f,1.f,0.f,.10f,.75f }};
    return p;
}
Patch patch(const char* name, std::initializer_list<std::pair<Param, float>> changes)
{
    Patch p = base(name);
    for (const auto& [id, value] : changes) p.values[id] = value;
    return p;
}
}

std::vector<Patch> Processor::makePresets()
{
    // These are original *recreations*, not copies of Roland's factory 128.
    return {
        patch("01  INIT - Classic Saw", {{cutoff,.82f},{chorus,0.f},{sub,0.f},{pulse,0.f}}),
        patch("02  Satin Strings", {{cutoff,.61f},{resonance,.14f},{attack,.45f},{release,.54f},{chorus,1.f},{sub,.42f},{pwm,.58f}}),
        patch("03  Velvet Brass", {{cutoff,.32f},{envAmount,.64f},{attack,.16f},{decay,.43f},{sustain,.44f},{release,.26f},{sub,.38f},{chorus,1.f}}),
        patch("04  Warm Poly Pad", {{cutoff,.53f},{attack,.56f},{release,.67f},{decay,.51f},{sustain,.77f},{pwm,.64f},{chorus,2.f},{sub,.38f}}),
        patch("05  1984 Bass", {{range,0.f},{saw,1.f},{pulse,1.f},{sub,.90f},{cutoff,.27f},{envAmount,.56f},{decay,.29f},{sustain,.14f},{release,.12f},{chorus,0.f}}),
        patch("06  Dreamy Chorus", {{cutoff,.58f},{resonance,.29f},{attack,.40f},{release,.72f},{pwm,.8f},{chorus,2.f},{sub,.55f}}),
        patch("07  Pure PWM", {{saw,0.f},{pulse,1.f},{sub,0.f},{cutoff,.80f},{pwm,.85f},{lfoRate,.31f},{chorus,1.f}}),
        patch("08  Sweet Pluck", {{saw,1.f},{pulse,0.f},{sub,.15f},{cutoff,.31f},{envAmount,.86f},{attack,0.f},{decay,.25f},{sustain,0.f},{release,.22f},{chorus,1.f}}),
        patch("09  Analog Choir", {{cutoff,.49f},{resonance,.15f},{attack,.61f},{release,.68f},{sustain,.83f},{pwm,.67f},{chorus,2.f},{noise,.07f}}),
        patch("10  Submarine", {{range,0.f},{cutoff,.22f},{resonance,.31f},{envAmount,.3f},{sub,1.f},{chorus,0.f},{sustain,.7f}}),
        patch("11  Big 80s Brass", {{cutoff,.38f},{envAmount,.82f},{attack,.10f},{decay,.34f},{sustain,.48f},{release,.18f},{vcaLevel,.92f},{sub,.28f}}),
        patch("12  Air Organ", {{range,1.f},{saw,0.f},{pulse,1.f},{sub,.64f},{cutoff,.76f},{vcaGate,1.f},{pwm,.50f},{chorus,1.f}}),
        patch("13  Round Electric", {{cutoff,.48f},{envAmount,.56f},{decay,.22f},{sustain,.10f},{release,.38f},{sub,.27f},{resonance,.26f}}),
        patch("14  Noisy Sweep", {{noise,.7f},{cutoff,.31f},{envAmount,.93f},{attack,.49f},{release,.58f},{chorus,2.f},{resonance,.60f}}),
        patch("15  Midnight Lead", {{range,1.f},{cutoff,.73f},{resonance,.21f},{sub,.32f},{pwmManual,1.f},{pwm,.41f},{portamento,.16f},{chorus,0.f}}),
        patch("16  Filter Whistle", {{saw,0.f},{pulse,0.f},{sub,0.f},{resonance,1.f},{cutoff,.43f},{keyTrack,1.f},{vcaGate,1.f},{chorus,0.f}}),
        patch("17  Glassy Bells", {{range,2.f},{cutoff,.61f},{resonance,.55f},{envAmount,.53f},{attack,.01f},{decay,.21f},{sustain,0.f},{release,.39f},{chorus,2.f}}),
        patch("18  PWM Cloud", {{saw,0.f},{pulse,1.f},{sub,.25f},{cutoff,.55f},{attack,.66f},{release,.73f},{lfoRate,.19f},{pwm,.83f},{chorus,2.f}}),
        patch("19  Short Synth Stab", {{cutoff,.29f},{envAmount,.90f},{attack,0.f},{decay,.18f},{sustain,.11f},{release,.12f},{sub,.38f},{chorus,0.f}}),
        patch("20  Hollow Flute", {{saw,0.f},{pulse,1.f},{sub,0.f},{noise,.12f},{cutoff,.50f},{attack,.21f},{release,.35f},{pwmManual,1.f},{pwm,.48f}}),
        patch("21  Deep Motion", {{range,0.f},{cutoff,.38f},{resonance,.52f},{filterLfo,.41f},{lfoRate,.29f},{sub,.55f},{chorus,1.f}}),
        patch("22  Resonant Keys", {{cutoff,.32f},{resonance,.67f},{envAmount,.62f},{decay,.30f},{sustain,.10f},{release,.29f},{chorus,1.f}}),
        patch("23  Soft Touch", {{saw,0.f},{sub,.30f},{cutoff,.62f},{attack,.22f},{release,.43f},{pwm,.32f},{chorus,1.f}}),
        patch("24  Burning Sunset", {{cutoff,.36f},{resonance,.29f},{envAmount,.39f},{attack,.50f},{release,.70f},{sub,.66f},{chorus,2.f}}),
        patch("25  Analog Piano", {{range,1.f},{cutoff,.51f},{envAmount,.55f},{attack,0.f},{decay,.40f},{sustain,.03f},{release,.31f},{sub,.25f},{chorus,1.f}}),
        patch("26  Brass Octaves", {{range,2.f},{cutoff,.39f},{envAmount,.75f},{attack,.09f},{decay,.39f},{sustain,.52f},{release,.20f},{sub,.59f},{chorus,0.f}}),
        patch("27  Warm Saw Lead", {{saw,1.f},{pulse,0.f},{cutoff,.66f},{resonance,.19f},{attack,.04f},{release,.26f},{chorus,1.f}}),
        patch("28  Soft Noise Pad", {{noise,.48f},{saw,1.f},{pulse,0.f},{cutoff,.48f},{attack,.63f},{release,.76f},{chorus,2.f},{sub,.13f}}),
        patch("29  Slow Resonance", {{cutoff,.39f},{resonance,.68f},{filterLfo,.45f},{lfoRate,.10f},{attack,.35f},{release,.55f},{chorus,1.f}}),
        patch("30  Classic Synth Pop", {{cutoff,.57f},{envAmount,.36f},{attack,.07f},{decay,.35f},{sustain,.68f},{release,.33f},{sub,.25f},{chorus,1.f}}),
        patch("31  Deep Bass Split", {{range,0.f},{pulse,1.f},{saw,0.f},{sub,1.f},{cutoff,.35f},{resonance,.18f},{envAmount,.45f},{decay,.20f},{sustain,.22f},{chorus,0.f}}),
        patch("32  Shimmer Horizon", {{range,2.f},{cutoff,.67f},{resonance,.33f},{envAmount,.31f},{attack,.54f},{release,.75f},{pwm,.8f},{chorus,2.f}})
    };
}

Processor::Processor()
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
                           .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "AURORA106", makeLayout()),
      patches(makePresets())
{
    setCurrentProgram(0);
}

void Processor::prepareToPlay(double fs, int /*blockSize*/)
{
    engine.prepare(fs);
    engine.pitchWheel(pitchPosition.load(std::memory_order_relaxed));
    engine.modulationWheel(modPosition.load(std::memory_order_relaxed));
}

void Processor::setPitchBendFromUI(int position) noexcept
{
    pitchPosition.store(juce::jlimit(0, 16383, position), std::memory_order_relaxed);
}

void Processor::setModulationFromUI(int position) noexcept
{
    modPosition.store(juce::jlimit(0, 127, position), std::memory_order_relaxed);
}
void Processor::releaseResources() { engine.allNotesOff(); }

bool Processor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    const auto output = layout.getMainOutputChannelSet();
    return (output == juce::AudioChannelSet::mono()
         || output == juce::AudioChannelSet::stereo())
         && layout.getMainInputChannelSet().isDisabled();
}

float Processor::value(Param id) const noexcept
{
    auto* valuePtr = parameters.getRawParameterValue(parameterIds[id]);
    return valuePtr != nullptr ? valuePtr->load() : 0.f;
}

std::array<float, paramCount> Processor::currentValues() const noexcept
{
    std::array<float, paramCount> values{};
    for (size_t i = 0; i < paramCount; ++i)
        values[i] = value(static_cast<Param>(i));
    return values;
}

Settings Processor::readSettings() const noexcept
{
    Settings s;
    s.lfoRate=value(lfoRate); s.lfoDelay=value(lfoDelay);
    s.range=(int)value(range); s.dcoLfo=value(dcoLfo); s.pwm=value(pwm);
    s.pwmManual=value(pwmManual)>.5f; s.pulse=value(pulse)>.5f; s.saw=value(saw)>.5f;
    s.sub=value(sub); s.noise=value(noise); s.hpf=(int)value(hpf);
    s.cutoff=value(cutoff); s.resonance=value(resonance);
    s.vcfEnv=value(envAmount); s.envInverse=value(envInverse)>.5f;
    s.vcfLfo=value(filterLfo); s.keyFollow=value(keyTrack);
    s.vcaGate=value(vcaGate)>.5f; s.vcaLevel=value(vcaLevel);
    s.attack=value(attack); s.decay=value(decay); s.sustain=value(sustain); s.release=value(release);
    s.chorus=(int)value(chorus); s.portamento=value(portamento);
    s.drift=value(drift); s.master=value(master);
    return s;
}

void Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    if (buffer.getNumChannels() < 1) return;
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);
    engine.setSettings(readSettings());
    // GUI changes are applied at the start of each block. Incoming MIDI below
    // overrides them sample-accurately and also updates the UI display state.
    engine.pitchWheel(pitchPosition.load(std::memory_order_relaxed));
    engine.modulationWheel(modPosition.load(std::memory_order_relaxed));
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    int cursor = 0;
    for (const auto metadata : midi)
    {
        const auto position = juce::jlimit(cursor, buffer.getNumSamples(), metadata.samplePosition);
        if (position > cursor)
        {
            engine.render(left + cursor, right ? right + cursor : nullptr, position - cursor);
            cursor = position;
        }
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) engine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff()) engine.noteOff(msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine.allNotesOff();
        else if (msg.isPitchWheel())
        {
            const int bend = msg.getPitchWheelValue();
            pitchPosition.store(bend, std::memory_order_relaxed);
            engine.pitchWheel(bend);
        }
        else if (msg.isController() && msg.getControllerNumber() == 1)
        {
            const int mod = msg.getControllerValue();
            modPosition.store(mod, std::memory_order_relaxed);
            engine.modulationWheel(mod);
        }
        else if (msg.isController() && msg.getControllerNumber() == 64)
            engine.sustainPedal(msg.getControllerValue() >= 64);
    }
    if (cursor < buffer.getNumSamples())
        engine.render(left + cursor, right ? right + cursor : nullptr,
                      buffer.getNumSamples() - cursor);
}

void Processor::loadValues(const std::array<float, paramCount>& values)
{
    for (size_t i = 0; i < paramCount; ++i)
        if (auto* param = parameters.getParameter(parameterIds[i]))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(param->convertTo0to1(values[i]));
            param->endChangeGesture();
        }
}

void Processor::setCurrentProgram(int index)
{
    if (index < 0 || index >= (int) patches.size()) return;
    selectedProgram = index;
    loadValues(patches[static_cast<size_t>(index)].values);
}

const juce::String Processor::getProgramName(int index)
{
    return programName(index);
}
juce::String Processor::programName(int index) const
{
    if (index < 0 || index >= (int) patches.size()) return "Custom";
    return patches[static_cast<size_t>(index)].name;
}

void Processor::refreshCustomPatchName()
{
    selectedProgram = -1;
}

void Processor::getStateInformation(juce::MemoryBlock& dest)
{
    auto tree = parameters.copyState();
    tree.setProperty("program", selectedProgram, nullptr);
    for (size_t j = 32; j < patches.size(); ++j)
    {
        juce::ValueTree child("IMPORTED_PATCH");
        child.setProperty("name", patches[j].name, nullptr);
        for (size_t i = 0; i < paramCount; ++i)
            child.setProperty(parameterIds[i], patches[j].values[i], nullptr);
        tree.addChild(child, -1, nullptr);
    }
    if (auto xml = tree.createXml()) copyXmlToBinary(*xml, dest);
}

void Processor::setStateInformation(const void* data, int bytes)
{
    if (auto xml = getXmlFromBinary(data, bytes))
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid() && tree.hasType("AURORA106"))
        {
            patches.resize(32);
            for (const auto child : tree)
                if (child.hasType("IMPORTED_PATCH") && patches.size() < 160)
                {
                    Patch p = base(child.getProperty("name").toString());
                    for (size_t i = 0; i < paramCount; ++i)
                        p.values[i] = (float) child.getProperty(parameterIds[i], p.values[i]);
                    patches.push_back(p);
                }
            selectedProgram = (int) tree.getProperty("program", -1);
            if (selectedProgram >= (int) patches.size()) selectedProgram = -1;
            parameters.replaceState(tree);
        }
    }
}

int Processor::importSysEx(const juce::File& file, juce::String& error)
{
    juce::MemoryBlock block;
    if (!file.loadFileAsData(block)) { error = "Could not read SysEx file"; return 0; }
    const auto* data = static_cast<const uint8_t*>(block.getData());
    const auto n = block.getSize();
    std::vector<Patch> imported;
    for (size_t i = 0; i + 23 < n; ++i)
    {
        if (data[i] != 0xF0 || data[i+1] != 0x41 || data[i+2] != 0x30
            || data[i+3] > 0x0F || data[i+23] != 0xF7) continue;
        bool good = true;
        for (size_t x = i+4; x < i+23; ++x) if (data[x] > 127) good = false;
        if (!good) continue;
        const uint8_t* d = data + i + 5;
        Patch p = base("Imported " + juce::String((int)imported.size()+1).paddedLeft('0', 3));
        auto& v = p.values;
        v[lfoRate]=d[0]/127.f; v[lfoDelay]=d[1]/127.f;
        v[dcoLfo]=d[2]/127.f; v[pwm]=d[3]/127.f;
        v[noise]=d[4]/127.f; v[cutoff]=d[5]/127.f;
        v[resonance]=d[6]/127.f; v[envAmount]=d[7]/127.f;
        v[filterLfo]=d[8]/127.f; v[keyTrack]=d[9]/127.f;
        // The hardware VCA slider is bipolar -5..+5; normalize approximately.
        v[vcaLevel]=d[10]/127.f;
        v[attack]=d[11]/127.f; v[decay]=d[12]/127.f;
        v[sustain]=d[13]/127.f; v[release]=d[14]/127.f;
        v[sub]=d[15]/127.f;
        const uint8_t b0=d[16], b1=d[17];
        v[chorus]=(b0 & 0x20) ? 0.f : ((b0 & 0x40) ? 1.f : 2.f);
        v[saw]=(b0 & 0x10) ? 1.f : 0.f;
        v[pulse]=(b0 & 0x08) ? 1.f : 0.f;
        v[range]=(b0 & 0x01) ? 0.f : ((b0 & 0x04) ? 2.f : 1.f);
        v[hpf]=(float) (3 - ((b1 >> 3) & 3));
        v[vcaGate]=(b1 & 0x04) ? 1.f : 0.f;
        v[envInverse]=(b1 & 0x02) ? 1.f : 0.f;
        v[pwmManual]=(b1 & 0x01) ? 1.f : 0.f;
        imported.push_back(p);
        i += 23;
    }
    if (imported.empty())
    {
        error = "No Juno-106 single-patch SysEx messages found (F0 41 30 ...)";
        return 0;
    }
    const auto available = (int) (160 - patches.size());
    const int count = std::min(available, (int) imported.size());
    if (count == 0) { error = "Bank is full (128 imported patches max)"; return 0; }
    for (int i = 0; i < count; ++i) patches.push_back(std::move(imported[(size_t)i]));
    setCurrentProgram((int) patches.size() - count);
    error = "Imported " + juce::String(count) + " patch(es).";
    return count;
}

bool Processor::exportPreset(const juce::File& file, juce::String& error)
{
    juce::ValueTree tree("AURORA_PATCH");
    tree.setProperty("name", selectedProgram < 0 ? "Custom Patch" : programName(selectedProgram), nullptr);
    for (size_t i = 0; i < paramCount; ++i)
        tree.setProperty(parameterIds[i], value(static_cast<Param>(i)), nullptr);
    if (auto xml = tree.createXml())
        if (file.replaceWithText(xml->toString())) { error = "Preset saved."; return true; }
    error = "Failed to write preset file.";
    return false;
}

bool Processor::importPreset(const juce::File& file, juce::String& error)
{
    auto xml = juce::XmlDocument::parse(file);
    if (!xml) { error = "Not a valid XML preset file."; return false; }
    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.hasType("AURORA_PATCH")) { error = "Not an Aurora 106 preset."; return false; }
    Patch p = base(tree.getProperty("name", "Imported Patch").toString());
    for (size_t i = 0; i < paramCount; ++i)
        p.values[i] = (float) tree.getProperty(parameterIds[i], p.values[i]);
    if (patches.size() >= 160) { error = "Maximum 128 imported presets."; return false; }
    patches.push_back(p);
    setCurrentProgram((int)patches.size() - 1);
    error = "Preset loaded.";
    return true;
}
} // namespace aurora

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new aurora::Processor();
}
