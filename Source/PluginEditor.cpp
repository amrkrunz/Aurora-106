#include "PluginEditor.h"
#include <cmath>

namespace aurora
{
namespace
{
    // JUCE's Colour(uint32) constructor is not constexpr in JUCE 8.
    // File-scope const values are fine; constexpr fails on MSVC (C2131).
    const juce::Colour panel  { 0xff25292e };
    const juce::Colour light  { 0xfff0eee2 };
    const juce::Colour muted  { 0xffa1aab3 };
    const juce::Colour orange { 0xfff2a846 };
    const juce::Colour red    { 0xffd34847 };
    const juce::Colour blue   { 0xff448abd };
    const juce::Colour rail   { 0xff101317 };
}

PanelLook::PanelLook()
{
    setColour(juce::ComboBox::textColourId, light);
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId, rail);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, light);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, orange);
    setColour(juce::PopupMenu::highlightedTextColourId, rail);
    setColour(juce::Label::textColourId, light);
    setColour(juce::TextButton::textColourOffId, light);
    setColour(juce::TextButton::textColourOnId, rail);
}

void PanelLook::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                 float sliderPos, float, float,
                                 const juce::Slider::SliderStyle style, juce::Slider&)
{
    if (style != juce::Slider::LinearVertical)
        return;
    const float center = x + width * .5f;
    const auto trackTop = y + 7.f, trackBottom = y + height - 9.f;
    const float trackHeight = trackBottom - trackTop;
    g.setColour(juce::Colours::black.withAlpha(.65f));
    g.fillRoundedRectangle(center - 5.f, trackTop, 10.f, trackHeight, 3.f);
    g.setColour(juce::Colour(0xff3d444d));
    g.drawRoundedRectangle(center - 5.f, trackTop, 10.f, trackHeight, 3.f, 1.f);
    for (int i = 0; i <= 10; ++i)
    {
        float markerY = trackTop + trackHeight * i / 10.f;
        g.setColour((i == 0 || i == 5 || i == 10) ? light.withAlpha(.72f) : muted.withAlpha(.32f));
        const float len = (i % 5 == 0) ? 9.f : 5.f;
        g.drawLine(center - 18.f, markerY, center - 18.f + len, markerY, 1.f);
        g.drawLine(center + 18.f - len, markerY, center + 18.f, markerY, 1.f);
    }
    const float handleY = juce::jlimit(trackTop, trackBottom, sliderPos);
    juce::DropShadow shadow(juce::Colours::black.withAlpha(.8f), 5, {0, 2});
    juce::Rectangle<int> bounds((int)(center - 16.f), (int)(handleY - 10.f), 32, 20);
    shadow.drawForRectangle(g, bounds);
    juce::ColourGradient grad(juce::Colour(0xff858d93), center, handleY - 10.f,
                              juce::Colour(0xff343a40), center, handleY + 10.f, false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds.toFloat(), 2.f);
    g.setColour(juce::Colour(0xff1e242a));
    g.drawRoundedRectangle(bounds.toFloat(), 2.f, 1.f);
    g.setColour(light.withAlpha(.95f));
    g.fillRect(center - 11.f, handleY - 1.f, 22.f, 2.f);
}

void PanelLook::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                float value, float start, float end, juce::Slider&)
{
    auto bounds = juce::Rectangle<float>((float)x+3.f, (float)y+3.f,
                                         (float)width-6.f, (float)height-6.f);
    auto centre = bounds.getCentre();
    float radius = bounds.getWidth() * 0.5f;
    g.setColour(juce::Colours::black.withAlpha(.6f));
    g.fillEllipse(bounds.translated(1.f, 2.f));
    g.setColour(juce::Colour(0xff626b72));
    g.drawEllipse(bounds, 3.f);
    bounds.reduce(4.f, 4.f);
    g.setColour(juce::Colour(0xff14191e));
    g.fillEllipse(bounds);
    const float angle = start + value * (end - start);
    juce::Path indicator;
    auto vec = juce::Point<float>(std::sin(angle), -std::cos(angle));
    indicator.startNewSubPath(centre.x + vec.x * 5.f, centre.y + vec.y * 5.f);
    indicator.lineTo(centre.x + vec.x * (radius - 8.f), centre.y + vec.y * (radius - 8.f));
    g.setColour(orange);
    g.strokePath(indicator, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
}

void PanelLook::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                    const juce::Colour&, bool highlighted, bool pressed)
{
    auto rect = button.getLocalBounds().toFloat().reduced(1.f);
    g.setColour(juce::Colours::black.withAlpha(.70f));
    g.fillRoundedRectangle(rect.translated(0.f, 2.f), 3.f);
    auto top = button.getToggleState() || pressed ? orange : juce::Colour(0xff3a434b);
    if (highlighted) top = top.brighter(.12f);
    g.setColour(top);
    g.fillRoundedRectangle(rect, 3.f);
    g.setColour(light.withAlpha(.14f));
    g.drawRoundedRectangle(rect, 3.f, 1.f);
}

void PanelLook::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                 bool highlighted, bool pressed)
{
    auto rect = button.getLocalBounds().toFloat().reduced(1.5f);
    auto on = button.getToggleState();
    g.setColour(juce::Colours::black.withAlpha(.75f));
    g.fillRoundedRectangle(rect.translated(0.f, 2.f), 2.f);
    juce::Colour top = on ? orange : juce::Colour(0xffdedacb);
    if (highlighted || pressed) top = top.brighter(.12f);
    g.setColour(top);
    g.fillRoundedRectangle(rect, 2.f);
    g.setColour(juce::Colour(0xff182029));
    g.setFont(juce::Font(juce::FontOptions(11.f).withStyle("Bold")));
    g.drawText(button.getButtonText(), rect.toNearestInt(), juce::Justification::centred);
    g.setColour(on ? red : juce::Colour(0xff494c4e));
    g.fillEllipse(rect.getRight()-7.f, rect.getY()+3.f, 4.f, 4.f);
}

void PanelLook::drawComboBox(juce::Graphics& g, int width, int height, bool,
                             int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float>(0.f, 0.f, (float)width, (float)height);
    g.setColour(rail);
    g.fillRoundedRectangle(r, 3.f);
    g.setColour(juce::Colour(0xff667680));
    g.drawRoundedRectangle(r.reduced(.5f), 3.f, 1.f);
    g.setColour(orange);
    juce::Path arrow;
    arrow.addTriangle((float)width-17.f, height*.38f, (float)width-7.f, height*.38f,
                      (float)width-12.f, height*.62f);
    g.fillPath(arrow);
}
void PanelLook::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(10, 0, box.getWidth()-35, box.getHeight());
    label.setFont(juce::Font(juce::FontOptions(12.f).withStyle("Bold")));
}

void PerformanceWheel::setNormalized(float v, bool notify)
{
    v = juce::jlimit(0.f, 1.f, v);
    if (std::abs(position - v) < .00001f) return;
    position = v;
    repaint();
    if (notify && onChange) onChange(position);
}

void PerformanceWheel::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced(3.f, 2.f);
    const float cx = area.getCentreX();
    g.setColour(juce::Colour(0xff090c0f));
    g.fillRoundedRectangle(area, 8.f);
    g.setColour(juce::Colour(0xff56616a));
    g.drawRoundedRectangle(area, 8.f, 1.5f);

    const float trackTop = area.getY() + 9.f;
    const float trackBottom = area.getBottom() - 9.f;
    const float cy = trackBottom - position * (trackBottom - trackTop);
    const float rollerH = 30.f;
    const juce::Rectangle<float> roller(area.getX() + 6.f, cy - rollerH * .5f,
                                         area.getWidth() - 12.f, rollerH);
    g.setColour(juce::Colours::black.withAlpha(.66f));
    g.fillRoundedRectangle(roller.translated(0.f, 2.f), 5.f);
    juce::ColourGradient rubber(juce::Colour(0xff697179), cx, roller.getY(),
                                juce::Colour(0xff1d2229), cx, roller.getBottom(), false);
    g.setGradientFill(rubber);
    g.fillRoundedRectangle(roller, 4.f);
    g.setColour(juce::Colour(0xff98a3a9).withAlpha(.65f));
    for (int i = 0; i < 5; ++i)
    {
        const float y = roller.getY() + 7.f + i * 4.f;
        g.drawLine(roller.getX() + 5.f, y, roller.getRight() - 5.f, y, .75f);
    }
    g.setColour(orange);
    g.fillRoundedRectangle(roller.getX() + 3.f, cy - 1.5f,
                           roller.getWidth() - 6.f, 3.f, 1.3f);
    if (springLoaded)
    {
        const float neutral = (trackTop + trackBottom) * .5f;
        g.setColour(light.withAlpha(.65f));
        g.drawLine(area.getX() - 2.f, neutral, area.getX() + 2.f, neutral, 1.f);
        g.drawLine(area.getRight() - 2.f, neutral, area.getRight() + 2.f, neutral, 1.f);
    }
}

void PerformanceWheel::updateFromMouse(const juce::MouseEvent& e)
{
    const float top = 14.f, bottom = static_cast<float>(getHeight()) - 14.f;
    setNormalized((bottom - e.position.y) / juce::jmax(1.f, bottom - top), true);
}

void PerformanceWheel::mouseDown(const juce::MouseEvent& e)
{
    dragging = true;
    updateFromMouse(e);
}
void PerformanceWheel::mouseDrag(const juce::MouseEvent& e) { updateFromMouse(e); }
void PerformanceWheel::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
    if (springLoaded) setNormalized(.5f, true);
}
void PerformanceWheel::mouseDoubleClick(const juce::MouseEvent&)
{
    setNormalized(springLoaded ? .5f : 0.f, true);
}

Editor::Editor(Processor& owner)
    : juce::AudioProcessorEditor(owner), processor(owner), tooltips(this, 500),
      keyboard(owner.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&look);
    setSize(1360, 700);
    setResizable(false, false);

    programs.setLookAndFeel(&look);
    programs.setBounds(470, 50, 395, 34);
    programs.onChange = [this]
    {
        if (programs.getSelectedId() > 0)
        {
            processor.setCurrentProgram(programs.getSelectedId()-1);
            updateStatus("LOADED: " + processor.programName(programs.getSelectedId()-1));
        }
    };
    addAndMakeVisible(programs);
    updatePrograms();

    previous.setBounds(427, 50, 35, 34);
    next.setBounds(873, 50, 35, 34);
    previous.onClick = [this]
    {
        if (processor.programCount() > 0)
            programs.setSelectedId(((processor.getCurrentProgram()+processor.programCount()-1)
                     % processor.programCount()) + 1);
    };
    next.onClick = [this]
    {
        programs.setSelectedId(((processor.getCurrentProgram()+1)
                    % processor.programCount()) + 1);
    };
    addAndMakeVisible(previous);
    addAndMakeVisible(next);

    loadSx.setBounds(935, 50, 120, 34);
    loadPreset.setBounds(1065, 50, 110, 34);
    savePatch.setBounds(1185, 50, 138, 34);
    loadSx.onClick = [this] { openImport(true); };
    loadPreset.onClick = [this] { openImport(false); };
    savePatch.onClick = [this] { savePreset(); };
    addAndMakeVisible(loadSx);
    addAndMakeVisible(loadPreset);
    addAndMakeVisible(savePatch);

    // Original top-panel arrangement: LFO | DCO | HPF | VCF | VCA | ENV | CHORUS.
    fader(lfoRate,"RATE","LFO / Triangle-rate, 0.1 to 30 Hz",42,180);
    fader(lfoDelay,"DELAY","LFO / Fade-in delay, 0 to 3 s",100,180);

    fader(dcoLfo,"LFO","DCO / Pitch modulation depth",187,180);
    fader(pwm,"PWM","DCO / Manual pulse width or LFO PWM depth",245,180);
    fader(sub,"SUB","DCO / Square sub-oscillator, one octave lower",330,180);
    fader(noise,"NOISE","DCO / White noise source level",397,180);
    toggle(pwmManual,"MAN",196,466,55);
    toggle(pulse,"PULSE",271,466,71);
    toggle(saw,"SAW",351,466,62);
    combo(range,{"16'", "8'", "4'"},421,466,55);

    fader(cutoff,"FREQ","VCF / 24 dB low-pass cutoff",588,180);
    fader(resonance,"RES","VCF / Resonance and self-oscillation",648,180);
    fader(envAmount,"ENV","VCF / ADSR envelope amount",708,180);
    fader(filterLfo,"LFO","VCF / Triangle-LFO amount",767,180);
    fader(keyTrack,"KYBD","VCF / Keyboard tracking",827,180);
    toggle(envInverse,"ENV -",690,466,75);

    fader(vcaLevel,"LEVEL","VCA / Final voice amplifier level",884,180);
    toggle(vcaGate,"GATE",899,466,71);

    fader(attack,"A","ENV / Attack up to 3 s",979,180);
    fader(decay,"D","ENV / Decay up to 12 s",1037,180);
    fader(sustain,"S","ENV / Sustain level",1095,180);
    fader(release,"R","ENV / Release up to 12 s",1153,180);

    // Highpass is 4-position in the original, represented as discrete switch.
    combo(hpf,{"0 BASS", "1 FLAT", "2 HIGH", "3 HIGH"},490,268,79);
    combo(chorus,{"OFF", "I", "II"},1220,265,106);

    knob(portamento,"PORTAMENTO","Polyphonic glide between notes",206,526);
    knob(drift,"VOICE DRIFT","Per-voice DCO variation: optional modern extension",315,526);
    knob(master,"OUTPUT","Master output volume",1193,526);

    status.setBounds(465, 542, 655, 28);
    status.setJustificationType(juce::Justification::centred);
    status.setFont(juce::Font(juce::FontOptions(13.f).withStyle("Bold")));
    status.setColour(juce::Label::textColourId, orange);
    status.setText("SIX VOICES  |  MIDI READY  |  1984 CIRCUIT-INSPIRED SOUND", juce::dontSendNotification);
    addAndMakeVisible(status);

    pitchWheel.setBounds(34, 614, 51, 69);
    pitchWheel.setNormalized(processor.pitchBendPosition() / 16384.f, false);
    pitchWheel.setTooltip("Pitch bend -2/+2 semitones; springs back to centre; MIDI pitch bend");
    pitchWheel.onChange = [this](float v)
    {
        processor.setPitchBendFromUI(juce::jlimit(0, 16383, (int) std::lround(v * 16384.f)));
        updateStatus("PITCH BEND  |  +/- 2 SEMITONES  |  SPRING RETURN");
    };
    addAndMakeVisible(pitchWheel);

    modWheel.setBounds(100, 614, 51, 69);
    modWheel.setNormalized(processor.modulationPosition() / 127.f, false);
    modWheel.setTooltip("Modulation wheel: MIDI CC1 / LFO vibrato, stays in place");
    modWheel.onChange = [this](float v)
    {
        processor.setModulationFromUI(juce::jlimit(0, 127, (int) std::lround(v * 127.f)));
        updateStatus("MOD WHEEL  |  CC1  |  LFO VIBRATO DEPTH");
    };
    addAndMakeVisible(modWheel);

    keyboard.setBounds(170, 595, 1175, 95);
    keyboard.setAvailableRange(36, 96);
    keyboard.setKeyWidth(32.f);
    keyboard.setScrollButtonsVisible(false);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, light);
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, rail);
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
                       juce::Colour(0xff30353b));
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, orange.withAlpha(.38f));
    addAndMakeVisible(keyboard);
    startTimerHz(30); // Update wheel display when played from an external MIDI controller.
}

Editor::~Editor()
{
    stopTimer();
    chooser.reset();
    // Attachments and JUCE components must die before the custom L&F.
    programs.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void Editor::fader(Param id, juce::String label, juce::String help, int x, int y, int h)
{
    FaderItem item;
    item.control = std::make_unique<HoverFader>(help);
    item.control->setSliderStyle(juce::Slider::LinearVertical);
    item.control->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.control->setRange(0, 1, 0.0001);
    item.control->setBounds(x, y, 43, h);
    item.control->setTooltip(help);
    item.control->onHover = [this](const juce::String& message) { updateStatus(message); };
    item.caption = std::make_unique<juce::Label>();
    item.caption->setText(label, juce::dontSendNotification);
    item.caption->setJustificationType(juce::Justification::centred);
    item.caption->setFont(juce::Font(juce::FontOptions(11.f).withStyle("Bold")));
    item.caption->setColour(juce::Label::textColourId, light);
    item.caption->setBounds(x-7, y-27, 57, 19);
    addAndMakeVisible(*item.control);
    addAndMakeVisible(*item.caption);
    item.attach = std::make_unique<SliderAttach>(processor.state(), parameterIds[id], *item.control);
    faders.emplace_back(std::move(item));
}

void Editor::knob(Param id, juce::String label, juce::String help, int x, int y)
{
    FaderItem item;
    item.control = std::make_unique<HoverFader>(help);
    item.control->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.control->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.control->setBounds(x, y, 44, 44);
    item.control->setTooltip(help);
    item.control->onHover = [this](const juce::String& message) { updateStatus(message); };
    item.caption = std::make_unique<juce::Label>();
    item.caption->setText(label, juce::dontSendNotification);
    item.caption->setJustificationType(juce::Justification::centred);
    item.caption->setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    item.caption->setBounds(x-24,y+43,92,18);
    addAndMakeVisible(*item.caption);
    addAndMakeVisible(*item.control);
    item.attach = std::make_unique<SliderAttach>(processor.state(), parameterIds[id], *item.control);
    faders.emplace_back(std::move(item));
}

void Editor::toggle(Param id, const juce::String& label, int x, int y, int w)
{
    SwitchItem item;
    item.control = std::make_unique<juce::ToggleButton>(label);
    item.control->setBounds(x,y,w,27);
    item.control->setTooltip(label);
    addAndMakeVisible(*item.control);
    item.attach = std::make_unique<ButtonAttach>(processor.state(), parameterIds[id], *item.control);
    switches.emplace_back(std::move(item));
}

void Editor::combo(Param id, const juce::StringArray& entries, int x, int y, int w)
{
    MenuItem item;
    item.control = std::make_unique<juce::ComboBox>();
    item.control->setBounds(x,y,w,29);
    for (int i = 0; i < entries.size(); ++i)
        item.control->addItem(entries[i], i + 1);
    addAndMakeVisible(*item.control);
    item.attach = std::make_unique<ComboAttach>(processor.state(), parameterIds[id], *item.control);
    menus.emplace_back(std::move(item));
}

void Editor::updatePrograms()
{
    const int originalId = processor.getCurrentProgram() + 1;
    programs.clear(juce::dontSendNotification);
    for (int i = 0; i < processor.programCount(); ++i)
        programs.addItem(processor.programName(i), i + 1);
    if (originalId > 0) programs.setSelectedId(originalId, juce::dontSendNotification);
    else programs.setText("CUSTOM / EDITED", juce::dontSendNotification);
}

void Editor::timerCallback()
{
    // MIDI messages update the atomic controls in the audio callback. Never
    // overwrite a wheel while the user is actively dragging it.
    if (!pitchWheel.isDragging())
        pitchWheel.setNormalized(processor.pitchBendPosition() / 16384.f, false);
    if (!modWheel.isDragging())
        modWheel.setNormalized(processor.modulationPosition() / 127.f, false);
}

void Editor::updateStatus(const juce::String& value)
{
    status.setText(value, juce::dontSendNotification);
}

void Editor::openImport(bool isSysEx)
{
    chooser = std::make_unique<juce::FileChooser>(isSysEx ? "Import Juno-106 SysEx patches"
                                                          : "Load Aurora 106 patch",
                                                  juce::File{}, isSysEx ? "*.syx;*.SYX" : "*.j106preset;*.xml");
    juce::Component::SafePointer<Editor> safe(this);
    chooser->launchAsync(juce::FileBrowserComponent::openMode
                       | juce::FileBrowserComponent::canSelectFiles,
                       [safe, isSysEx](const juce::FileChooser& selected)
                       {
                           if (safe == nullptr) return;
                           const auto file = selected.getResult();
                           if (!file.existsAsFile()) return;
                           juce::String result;
                           if (isSysEx)
                               safe->processor.importSysEx(file, result);
                           else
                               safe->processor.importPreset(file, result);
                           safe->updatePrograms();
                           safe->updateStatus(result);
                       });
}

void Editor::savePreset()
{
    chooser = std::make_unique<juce::FileChooser>("Save Aurora 106 patch",
                    juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                        .getChildFile("My Aurora Patch.j106preset"),
                    "*.j106preset");
    juce::Component::SafePointer<Editor> safe(this);
    chooser->launchAsync(juce::FileBrowserComponent::saveMode
                         | juce::FileBrowserComponent::canSelectFiles
                         | juce::FileBrowserComponent::warnAboutOverwriting,
                         [safe](const juce::FileChooser& selected)
                         {
                             if (safe == nullptr) return;
                             auto file = selected.getResult();
                             if (file == juce::File{}) return;
                             if (!file.hasFileExtension(".j106preset"))
                                 file = file.withFileExtension(".j106preset");
                             juce::String result;
                             safe->processor.exportPreset(file, result);
                             safe->updateStatus(result);
                         });
}

void Editor::paint(juce::Graphics& g)
{
    g.fillAll(panel);
    juce::ColourGradient header(juce::Colour(0xff171b20), 0.f, 0.f,
                                 juce::Colour(0xff2b333a), 0.f, 120.f, false);
    g.setGradientFill(header);
    g.fillRect(0, 0, getWidth(), 118);
    g.setColour(juce::Colour(0xff131619));
    g.fillRect(0, 494, getWidth(), 91);
    g.setColour(red);
    g.fillRect(0, 112, getWidth(), 6);
    g.setColour(blue);
    g.fillRect(0, 585, getWidth(), 6);

    g.setColour(light);
    g.setFont(juce::Font(juce::FontOptions(37.f).withStyle("Bold")));
    g.drawText("AURORA", 25, 29, 226, 50, juce::Justification::centredLeft);
    g.setColour(orange);
    g.drawText("106", 259, 29, 91, 50, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(11.f).withStyle("Bold")));
    g.drawText("PROGRAMMABLE POLYPHONIC SYNTHESIZER", 29, 84, 330, 20,
               juce::Justification::centredLeft);
    g.drawText("PATCH MEMORY", 476, 29, 230, 16, juce::Justification::centredLeft);
    g.drawText("ANALOG MODEL  /  SIX VOICES", 1120, 24, 208, 20,
               juce::Justification::centredRight);

    struct Part { const char* name; int x; int width; };
    const Part parts[] = {
        {"LFO", 20, 155}, {"DCO", 176, 305}, {"HPF", 482, 93},
        {"VCF", 576, 300}, {"VCA", 877, 97}, {"ENV", 975, 233},
        {"CHORUS", 1209, 130}
    };
    for (const auto& p : parts)
    {
        g.setColour(red);
        g.fillRect(p.x, 138, p.width, 24);
        g.setColour(juce::Colour(0xff202428));
        g.setFont(juce::Font(juce::FontOptions(15.f).withStyle("Bold")));
        g.drawText(p.name, p.x + 12, 139, p.width-19, 23, juce::Justification::centredLeft);
        g.setColour(light.withAlpha(.18f));
        g.drawVerticalLine(p.x+p.width, 139.f, 495.f);
    }

    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(12.f).withStyle("Bold")));
    g.drawText("FOUR-POSITION", 491, 224, 83, 17, juce::Justification::centred);
    g.drawText("HIGH PASS", 491, 243, 83, 17, juce::Justification::centred);
    g.drawText("STEREO BBD", 1216, 219, 110, 17, juce::Justification::centred);
    g.drawText("CHORUS", 1216, 237, 110, 17, juce::Justification::centred);
    g.setColour(orange);
    g.fillEllipse(1270, 313, 10.f, 10.f);
    g.setColour(light.withAlpha(.66f));
    g.setFont(juce::Font(juce::FontOptions(11.f)));
    g.drawText("MODE I / II", 1219, 338, 113, 17, juce::Justification::centred);
    g.drawText("DCO  -  VCF  -  VCA", 1219, 370, 113, 17, juce::Justification::centred);

    g.setColour(light);
    g.setFont(juce::Font(juce::FontOptions(16.f).withStyle("Bold")));
    g.drawText("PERFORMANCE", 30, 516, 159, 30, juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    g.drawText("PITCH", 28, 592, 63, 18, juce::Justification::centred);
    g.drawText("MOD", 94, 592, 63, 18, juce::Justification::centred);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(11.f)));
    g.drawText("INDEPENDENT AUDIO  /  JUNO-INSPIRED MODEL  /  NOT AFFILIATED WITH ROLAND",
               578, 565, 630, 18, juce::Justification::centred);
}

void Editor::resized()
{
    // Fixed-size front panel; all controls are placed in physical panel coordinates.
}
} // namespace aurora

juce::AudioProcessorEditor* aurora::Processor::createEditor()
{
    return new aurora::Editor(*this);
}
