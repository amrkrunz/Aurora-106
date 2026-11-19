#include "SynthEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
int main()
{
    aurora::SynthEngine synth;
    synth.prepare(48000);
    aurora::Settings s;
    s.chorus = 1;
    synth.setSettings(s);
    synth.noteOn(60, 1.f);
    std::vector<float> l(48000), r(48000);
    synth.render(l.data(), r.data(), (int) l.size());
    double energy = 0.0, width = 0.0;
    for (size_t i = 0; i < l.size(); ++i)
    {
        assert(std::isfinite(l[i]) && std::isfinite(r[i]));
        energy += l[i] * l[i];
        width += std::abs(l[i] - r[i]);
    }
    assert(energy > 0.01);
    assert(width > 0.01);
    synth.noteOff(60);
    std::fill(l.begin(), l.end(), 0.f);
    std::fill(r.begin(), r.end(), 0.f);
    synth.render(l.data(), r.data(), (int) l.size());
    synth.allNotesOff();

    // Compare identical deterministic voices with and without wheel modulation.
    // This checks that wheels actually affect generated samples, not just the GUI.
    auto wheelRender = [](int bend, int mod)
    {
        aurora::SynthEngine e;
        e.prepare(48000);
        aurora::Settings x;
        x.chorus = 0;
        x.dcoLfo = 0;
        x.lfoRate = .55f;
        e.setSettings(x);
        e.pitchWheel(bend);
        e.modulationWheel(mod);
        e.noteOn(60, 1.f);
        std::vector<float> left(12000), right(12000);
        e.render(left.data(), right.data(), (int)left.size());
        return left;
    };
    const auto plain = wheelRender(8192, 0);
    const auto bent = wheelRender(16383, 0);
    const auto modulated = wheelRender(8192, 127);
    double pitchDifference = 0., modDifference = 0.;
    for (size_t i = 0; i < plain.size(); ++i)
    {
        assert(std::isfinite(modulated[i]) && std::isfinite(bent[i]));
        pitchDifference += std::abs(plain[i] - bent[i]);
        modDifference += std::abs(plain[i] - modulated[i]);
    }
    assert(pitchDifference > 1.0 && modDifference > 1.0);
    std::cout << "DSP OK: energy=" << energy << " stereo_width=" << width << " pitch_delta=" << pitchDifference
              << " mod_delta=" << modDifference << '\n';
}
