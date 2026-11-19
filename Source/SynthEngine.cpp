#include "SynthEngine.h"
#include <algorithm>

namespace aurora
{
namespace
{
    constexpr double pi = 3.1415926535897932384626433832795;
    double clampd(double v, double a, double b) noexcept { return std::max(a, std::min(b, v)); }
    double frac(double x) noexcept { return x - std::floor(x); }
    double envSeconds(float position, double end) noexcept
    {
        return 0.0015 * std::pow(end / 0.0015, clampd(position, 0.0, 1.0));
    }
    double onePole(double t, double fs) noexcept
    {
        return 1.0 - std::exp(-6.90775527898 / std::max(1.0, t * fs));
    }
}

double SynthEngine::frequency(int midiNote) noexcept
{
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

double SynthEngine::triangle(double p) noexcept
{
    return 1.0 - 4.0 * std::abs(p - 0.5);
}

double SynthEngine::polyBlep(double t, double dt) noexcept
{
    if (dt <= 0) return 0;
    if (t < dt) { t /= dt; return t + t - t * t - 1.0; }
    if (t > 1.0 - dt) { t = (t - 1.0) / dt; return t * t + t + t + 1.0; }
    return 0;
}

uint32_t SynthEngine::nextRandom() noexcept
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

void SynthEngine::prepare(double newRate)
{
    sampleRate = std::max(8000.0, newRate);
    reset();
}

void SynthEngine::reset()
{
    voices = {};
    chorus.clear();
    ageCounter = 0;
    lfoPhase = lfoElapsed = 0;
    lastHz = 0;
    pitchRatio = 1.0;
    modulationDepth = 0.0;
    pedalDown = false;
}

void SynthEngine::setSettings(const Settings& incoming) noexcept
{
    settings = incoming;
    settings.range = std::clamp(settings.range, 0, 2);
    settings.hpf = std::clamp(settings.hpf, 0, 3);
    settings.chorus = std::clamp(settings.chorus, 0, 2);
}

void SynthEngine::Voice::start(int key, double target, double initial,
                              uint64_t newAge, double driftCents) noexcept
{
    active = keyDown = true;
    deferredOff = false;
    note = key;
    age = newAge;
    targetHz = target;
    hz = initial;
    centsOffset = driftCents;
    // Retain running oscillator phases on retrigger (free-running DCO).
    lp[0] = lp[1] = lp[2] = lp[3] = 0.0;
    hpMemory = gateLevel = dcMemory = 0.0;
    env.level = 0.0;
    env.on();
}

void SynthEngine::Voice::releaseKey() noexcept
{
    keyDown = false;
    deferredOff = false;
    env.off();
}

void SynthEngine::noteOn(int note, float /*velocity*/)
{
    if (note < 0 || note > 127) return;
    if (activeVoices() == 0)
        lfoElapsed = 0.0;

    Voice* choice = nullptr;
    for (auto& v : voices)
        if (v.active && v.note == note && v.keyDown) { choice = &v; break; }
    if (choice == nullptr)
        for (auto& v : voices) if (!v.active) { choice = &v; break; }
    if (choice == nullptr)
        for (auto& v : voices)
            if (!v.keyDown && (choice == nullptr || v.age < choice->age)) choice = &v;
    if (choice == nullptr)
        choice = &*std::min_element(voices.begin(), voices.end(),
                  [](const Voice& a, const Voice& b) { return a.age < b.age; });

    const auto target = frequency(note);
    const auto init = (settings.portamento > 0.002f && lastHz > 0) ? lastHz : target;
    const double drift = (((nextRandom() & 0xffff) / 32767.5) - 1.0)
                       * static_cast<double>(settings.drift) * 4.0;
    choice->start(note, target, init, ++ageCounter, drift);
    lastHz = target;
}

void SynthEngine::noteOff(int note)
{
    for (auto& v : voices)
        if (v.active && v.note == note && v.keyDown)
        {
            if (pedalDown) { v.keyDown = false; v.deferredOff = true; }
            else v.releaseKey();
        }
}

void SynthEngine::sustainPedal(bool pressed)
{
    pedalDown = pressed;
    if (!pedalDown)
        for (auto& v : voices) if (v.deferredOff) v.releaseKey();
}

void SynthEngine::pitchWheel(int value14)
{
    const double semitones = 2.0 * (clampd(value14, 0, 16383) - 8192.0) / 8192.0;
    pitchRatio = std::pow(2.0, semitones / 12.0);
}

void SynthEngine::modulationWheel(int value7)
{
    modulationDepth = clampd(value7, 0, 127) / 127.0;
}

void SynthEngine::allNotesOff()
{
    pedalDown = false;
    for (auto& v : voices) if (v.active) v.releaseKey();
}

int SynthEngine::activeVoices() const noexcept
{
    int n = 0;
    for (const auto& v : voices) n += v.active ? 1 : 0;
    return n;
}

double SynthEngine::Envelope::tick(double fs, const Settings& s) noexcept
{
    switch (stage)
    {
        case Stage::idle: return 0.0;
        case Stage::attack:
            level += (1.001 - level) * onePole(envSeconds(s.attack, 3.0), fs);
            if (level >= 0.999) { level = 1.0; stage = Stage::decay; }
            break;
        case Stage::decay:
            level += (s.sustain - level) * onePole(envSeconds(s.decay, 12.0), fs);
            if (std::abs(level - s.sustain) < 0.0005) stage = Stage::sustain;
            break;
        case Stage::sustain: level = s.sustain; break;
        case Stage::release:
            level += (0.0 - level) * onePole(envSeconds(s.release, 12.0), fs);
            if (level < 0.00002) { level = 0.0; stage = Stage::idle; }
            break;
    }
    return level;
}

double SynthEngine::Voice::tick(double fs, const Settings& s, double lfo,
                                 double bend, double modulation) noexcept
{
    if (!active) return 0.0;
    const auto e = env.tick(fs, s);
    if (env.stage == Envelope::Stage::idle) { active = false; return 0.0; }

    const double glideSecs = 0.001 + 1.8 * s.portamento * s.portamento;
    const double portCoeff = s.portamento < 0.002 ? 1.0 : 1.0 - std::exp(-1.0 / (glideSecs * fs));
    hz += (targetHz - hz) * portCoeff;
    const double octaveShift = 1 - s.range; // 16' = -1, 8' = 0, 4' = +1
    // CC1 adds up to +/- 2 semitones of LFO vibrato without changing the patch.
    const double pitchLfoSemis = lfo * (s.dcoLfo + modulation) * 2.0;
    const double currentHz = hz * bend * std::pow(2.0,
                            octaveShift + (pitchLfoSemis + centsOffset / 100.0) / 12.0);
    const double duty = clampd(s.pwmManual ? 0.05 + 0.90 * s.pwm
                         : 0.50 + lfo * s.pwm * 0.45, 0.05, 0.95);

    // The Juno's HPF sits before the resonant VCF. Position zero is bass-enhancing.
    const double hpHz[] = { 170.0, 16.0, 110.0, 420.0 };
    const double hpCoeff = 1.0 - std::exp(-2.0 * pi * hpHz[s.hpf] / (fs * 2.0));

    // The same ADSR modulates the filter, and either that ADSR or the gate opens the VCA.
    double cutoff = 28.0 * std::pow(2.0, s.cutoff * 9.35);
    const double envOctaves = (s.envInverse ? -1.0 : 1.0) * 6.0 * s.vcfEnv * e;
    const double lfoOctaves = 2.0 * s.vcfLfo * lfo;
    const double keyboardOctaves = (note - 60) / 12.0 * s.keyFollow;
    cutoff *= std::pow(2.0, envOctaves + lfoOctaves + keyboardOctaves);
    cutoff = clampd(cutoff, 15.0, fs * 0.43);
    const double g = std::tan(pi * cutoff / (fs * 2.0));
    const double G = g / (1.0 + g);
    const double feedback = 4.10 * std::pow(s.resonance, 1.12);
    const double dt = clampd(currentHz / (fs * 2.0), 0.0, 0.45);

    double output = 0.0;
    for (int os = 0; os < 2; ++os)
    {
        const double p = phase;
        double osc = 0.0;
        if (s.saw)
            osc += (2.0 * p - 1.0) - SynthEngine::polyBlep(p, dt);
        if (s.pulse)
        {
            double square = (p < duty) ? 1.0 : -1.0;
            square += SynthEngine::polyBlep(p, dt);
            square -= SynthEngine::polyBlep(frac(p - duty + 1.0), dt);
            osc += square * 0.85;
        }
        if (s.sub > 0.0f)
        {
            const double subDT = dt * 0.5;
            double sub = (subPhase < 0.5) ? 1.0 : -1.0;
            sub += SynthEngine::polyBlep(subPhase, subDT);
            sub -= SynthEngine::polyBlep(frac(subPhase + 0.5), subDT);
            osc += sub * s.sub * 0.95;
        }
        // Noise has a dedicated shared source added in the upper render loop.
        osc *= 0.31;
        if (s.hpf == 0)
        {
            hpMemory += hpCoeff * (osc - hpMemory);
            osc += 0.23 * hpMemory;
        }
        else
        {
            hpMemory += hpCoeff * (osc - hpMemory);
            osc -= hpMemory;
        }

        // Four cascaded one-pole OTA-like integrators with saturating feedback.
        // This is a practical circuit-inspired approximation, not a SPICE model.
        const double input = std::tanh(osc * 1.45 - lp[3] * feedback);
        lp[0] += G * (input - lp[0]);
        lp[1] += G * (lp[0] - lp[1]);
        lp[2] += G * (lp[1] - lp[2]);
        lp[3] += G * (lp[2] - lp[3]);
        output += lp[3];
        phase = frac(phase + dt);
        subPhase = frac(subPhase + dt * 0.5);
    }
    output *= 0.5;

    const double targetGate = keyDown || deferredOff ? 1.0 : 0.0;
    gateLevel += (targetGate - gateLevel) * (1.0 - std::exp(-1.0 / (0.004 * fs)));
    const double amp = s.vcaGate ? gateLevel : e;
    // Reduce DC from high-resonance/self-oscillating settings without killing the bass.
    dcMemory += (output - dcMemory) * (1.0 - std::exp(-2.0 * pi * 7.0 / fs));
    return (output - dcMemory) * amp * (0.20 + s.vcaLevel * 0.95);
}

void SynthEngine::Chorus::clear() noexcept
{
    left.fill(0.0); right.fill(0.0);
    position = 0;
    phase = preL = preR = wetL = wetR = 0.0;
}

double SynthEngine::Chorus::read(const std::array<double, size>& buffer,
                                  double delaySamples) const noexcept
{
    double at = position - delaySamples;
    while (at < 0.0) at += size;
    const auto i0 = static_cast<int>(at) & (size - 1);
    const auto i1 = (i0 + 1) & (size - 1);
    return buffer[static_cast<size_t>(i0)] * (1.0 - frac(at))
         + buffer[static_cast<size_t>(i1)] * frac(at);
}

double SynthEngine::Chorus::processL(double in, double fs, int mode, double hiss) noexcept
{
    left[static_cast<size_t>(position)] = in;
    if (mode == 0) return in;
    const double phaseWave = std::sin(phase * 2.0 * pi);
    const double delayMs = mode == 1 ? 7.4 + 1.35 * phaseWave : 8.7 + 3.05 * phaseWave;
    double wet = read(left, fs * delayMs * 0.001);
    wetL += (wet - wetL) * (1.0 - std::exp(-2.0 * pi * 7600.0 / fs));
    return 0.75 * in + 0.53 * wetL + hiss;
}

double SynthEngine::Chorus::processR(double in, double fs, int mode, double hiss) noexcept
{
    right[static_cast<size_t>(position)] = in;
    if (mode == 0) return in;
    const double phaseWave = std::sin(phase * 2.0 * pi);
    const double delayMs = mode == 1 ? 7.4 - 1.35 * phaseWave : 8.7 - 3.05 * phaseWave;
    double wet = read(right, fs * delayMs * 0.001);
    wetR += (wet - wetR) * (1.0 - std::exp(-2.0 * pi * 7600.0 / fs));
    return 0.75 * in + 0.53 * wetR + hiss;
}

void SynthEngine::Chorus::advance(double fs, int mode) noexcept
{
    position = (position + 1) & (size - 1);
    const double rate = mode == 2 ? 0.78 : 0.45;
    phase = frac(phase + rate / fs);
}

void SynthEngine::render(float* left, float* right, int numSamples) noexcept
{
    for (int i = 0; i < numSamples; ++i)
    {
        lfoPhase = frac(lfoPhase + (0.1 * std::pow(300.0, settings.lfoRate)) / sampleRate);
        lfoElapsed += 1.0 / sampleRate;
        const double delay = settings.lfoDelay * 3.0;
        const double lfoFade = clampd((lfoElapsed - delay + 0.10) / 0.10, 0.0, 1.0);
        const double lfo = triangle(lfoPhase) * lfoFade;
        const double white = (static_cast<int32_t>(nextRandom()) / 2147483648.0);
        double mono = 0.0;
        for (auto& v : voices)
            if (v.active)
            {
                const double voice = v.tick(sampleRate, settings, lfo, pitchRatio, modulationDepth);
                // Noise is independently gated by each voice's VCA envelope.
                const double amp = settings.vcaGate ? v.gateLevel : v.env.level;
                mono += voice + white * settings.noise * 0.045 * amp;
            }

        // Consistent level at 1 to 6 active notes; unlike strict normalization,
        // this retains the natural swell as a chord gets larger.
        mono *= 0.71;
        const double hiss = (settings.chorus == 0 ? 0.0 : white * 0.000025);
        const double l = chorus.processL(mono, sampleRate, settings.chorus, hiss);
        const double r = chorus.processR(mono, sampleRate, settings.chorus, -hiss);
        chorus.advance(sampleRate, settings.chorus);
        const double level = 0.85 * settings.master;
        if (left) left[i] += static_cast<float>(std::tanh(l * level));
        if (right) right[i] += static_cast<float>(std::tanh(r * level));
    }
}
} // namespace aurora
