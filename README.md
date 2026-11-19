# Aurora 106 — six-voice Juno-inspired VST3 synthesizer

An independent, **source-built**, 1980s Roland JUNO-106-inspired subtractive synthesizer with a hardware-style panel. It is not an official Roland product and it is **not a measured, exact 1:1 circuit model**. The DSP is fully implemented, not a mock-up or an effect shell.

## Features

- Six voices with voice allocation, sustain pedal, note release, MIDI pitch bend and on-screen keyboard.
- Bandlimited pulse and sawtooth DCOs, octave-down square sub oscillator and noise source.
- Switchable 16'/8'/4' range, LFO/manual PWM, pitch LFO, four-position HPF (position 0 approximates original bass enhancement).
- Resonant four-pole 24 dB/oct low-pass filter with saturation, inverse/normal envelope modulation, LFO modulation, keyboard follow; filter processed at 2x rate.
- Single ADSR (filter and VCA), with VCA gate / envelope selection.
- Two modeled stereo modulated-delay chorus modes, with gentle high-frequency smoothing and low-level hiss.
- Optional voice drift and polyphonic portamento. These are **extensions** to the modeled path, and drift defaults low.
- **Two performance wheels** beside the piano keyboard: a spring-centered pitch bend wheel (±2 semitones), and a position-holding modulation wheel (up to ±2 semitones of LFO vibrato). Both can be operated with a mouse.
- Incoming **14-bit MIDI pitch bend** and **CC1 modulation** automatically control the wheels and are reflected on screen; a mouse drag controls the same DSP settings. Other MIDI (including CC64 sustain) keeps working. Wheels are live performance states, not saved into presets.
- **32 embedded, original programmed patches**, selected from the DAW's program list or top-panel browser. These are NOT Roland's 128 historical factory programs.
- Import **standard JUNO-106 single-patch SysEx messages** (`F0 41 30 ... 18 data bytes ... F7`) from `.syx` files. If the file contains multiple concatenated messages, up to 128 patches can be imported into the plugin's additional bank. Imported programs and parameters are persisted with DAW project state.
- Import/export independent `.j106preset` XML patches; no external preset packs or sample files are required.

## Download your compiled plugin — **no local compiling needed**

1. Create a **new, empty public GitHub repository** (e.g. `aurora-106`).
2. Extract this entire ZIP and upload **everything** in the `Aurora106` folder to the repository's root, **including the hidden `.github/workflows/build.yml`** folder. Commit to `main`.
3. Open the repository's **Actions** tab and select **Build Aurora 106**. It runs on push, or select **Run workflow**.
4. Open the successful workflow run and download the **Windows x64** or **macOS universal** artifact ZIP.
5. Copy the VST3 bundle to your plugin folder and rescan the plugins in your DAW:
   - **macOS:** `~/Library/Audio/Plug-Ins/VST3/` (or `/Library/Audio/Plug-Ins/VST3/`)
   - **Windows:** `C:\Program Files\Common Files\VST3\`
   - AU is also built for macOS, and a Standalone app is built for both platforms.

### macOS security

The macOS GitHub Actions artifacts are **unsigned and unnotarized**. Gatekeeper may prevent them from loading. If you trust the source and build, you can remove the quarantine flag on the extracted bundle you installed (in Terminal):

```bash
xattr -dr com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/Aurora 106.vst3"
```

For distribution to other Mac users, use your own Apple Developer ID and notarization workflow. On Apple Silicon macOS builds are universal arm64 + x86_64.

## Load the original JUNO-106 factory presets

Roland's original **128 factory presets are NOT redistributed here**. Roland currently offers them as the *JUNO-106: The Original 128* collection. This plugin doesn't read Roland Cloud's proprietary preset format. It can import **standard per-patch JUNO-106 SysEx** from a properly acquired `*.syx` file; the SysEx message layout comes from the original public MIDI implementation. **Tape-backup audio files, Juno-60 dumps, JU-06 files, and unrelated bulk dumps are not directly supported.**

To import: click **IMPORT SYX**, select your `.syx`, and choose the new `Imported ###` sounds in the top-panel patch menu. Users are responsible for obtaining patch data they have the right to use. Conversion from factory parameter values to DSP values is approximate because the hardware's control curves have not been fully measured.

## Build manually (optional)

CMake >=3.22, C++17 compiler, Git access, VS2022 Build Tools or Xcode required.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 3
ctest --test-dir build -C Release --output-on-failure
```

CMake automatically fetches JUCE 8.0.13. The standalone C++ DSP test checks finite generated audio, stereo chorus, and that both performance wheels measurably change the audio; it does not prove that the complete JUCE VST3 builds or that sound matches reference hardware.

## Sound authenticity and known limitations

This is a **functional Juno-106-style instrument**, not a laboratory-validated emulation. Its four OTA-like low-pass stages, DCOs, chorus delay and envelopes use engineering approximations; details such as 80017A chip response, DCO master-clock quantization, the exact hardware envelope timing tables, HPF curves, and BBD noise/companding are not yet fully calibrated. The filter may sound different from actual hardware, especially at high resonance. As with the original hardware, incoming note velocity is not used as an amplitude modulator. An exact recreation would require capturing reference recordings, tuning measured response curves and comparing matched SysEx settings against a serviced JUNO-106.

### Licensing

The custom Aurora 106 source files are MIT licensed. The build also downloads JUCE, which is third-party software with **GPLv3/commercial dual-licensing**: use and distribute the combined plugin only in accordance with applicable JUCE terms (for example GPLv3 distribution including source, or a commercial JUCE license). The brand and model names Roland / JUNO-106 belong to their owners and are used here only for descriptive compatibility and historical reference.

## References

- Roland JUNO-106 owner's manual: https://cdn.roland.com/assets/media/pdf/JUNO-106_OM.pdf
- Roland technical specifications: https://support.roland.com/hc/en-us/articles/201966419-JUNO-106-Technical-Specifications
- Historical single-patch MIDI data: https://www.atarimagazines.com/st-log/issue33/64_1_THE_ST_AND_THE_ROLAND_JUNO_106_SYNTHESIZER.php
- Roland original factory programs: https://www.roland.com/id/products/rc_juno-106_the_original_128/


## Windows compiler fix (19 November 2026)

The 2026-11-19 Windows GitHub Actions log reported `C2131` at
`Source/PluginEditor.cpp` lines 8–14. JUCE's `Colour(uint32)` constructor is
not a C++ constant expression under MSVC, so those seven file-scope
`constexpr auto` colour declarations were changed to `const juce::Colour`.
The unnecessary `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` flag was also removed
from the CI commands to eliminate misleading CMake warnings. The unrelated
JUCE splash-screen warning may still appear; it is nonfatal.

This patch addresses the compile error shown in the submitted Windows log.
A complete Windows and macOS JUCE plug-in build must still be confirmed by
rerunning GitHub Actions on the updated repository.


### Windows DSP smoke-test crash fix (2026-11-19)
The standalone DSP smoke test now heap-allocates the SynthEngine instances, avoiding a thread-stack overflow when the chorus delay buffers exceed the Windows default stack budget. The previous test was reproduced crashing with a 1 MiB stack limit; the corrected test passes under the same limit. This change affects the test harness, not synthesizer audio behavior.
