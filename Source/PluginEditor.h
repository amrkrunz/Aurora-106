#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <functional>

namespace aurora
{
class PanelLook final : public juce::LookAndFeel_V4
{
public:
    PanelLook();
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle, juce::Slider&) override;
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool highlighted, bool down) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isDown,
                      int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                      juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

class HoverFader final : public juce::Slider
{
public:
    explicit HoverFader(juce::String descriptiveName) : description(std::move(descriptiveName)) {}
    std::function<void(const juce::String&)> onHover;
    void mouseEnter(const juce::MouseEvent& e) override
    {
        juce::Slider::mouseEnter(e);
        if (onHover) onHover(description);
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (onHover) onHover(description);
        juce::Slider::mouseDown(e);
    }
private:
    juce::String description;
};

// Vertical wheel: pitch springs to centre on release; mod stays put.
// A drawn roller is used instead of a stock fader to match the hardware panel.
class PerformanceWheel final : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit PerformanceWheel(bool pitch) : springLoaded(pitch) {}
    std::function<void(float)> onChange;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void setNormalized(float position, bool notify);
    float getNormalized() const noexcept { return position; }
    bool isDragging() const noexcept { return dragging; }
private:
    void updateFromMouse(const juce::MouseEvent&);
    const bool springLoaded;
    float position = 0.0f; // mod 0; pitch centre set on construction by Editor
    bool dragging = false;
};

class Editor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Editor(Processor&);
    ~Editor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct FaderItem
    {
        std::unique_ptr<HoverFader> control;
        std::unique_ptr<juce::Label> caption;
        std::unique_ptr<SliderAttach> attach;
    };
    struct SwitchItem
    {
        std::unique_ptr<juce::ToggleButton> control;
        std::unique_ptr<ButtonAttach> attach;
    };
    struct MenuItem
    {
        std::unique_ptr<juce::ComboBox> control;
        std::unique_ptr<ComboAttach> attach;
    };
    void fader(Param, juce::String label, juce::String help, int x, int y, int h = 255);
    void knob(Param, juce::String label, juce::String help, int x, int y);
    void toggle(Param, const juce::String& label, int x, int y, int w = 66);
    void combo(Param, const juce::StringArray& items, int x, int y, int w);
    void updatePrograms();
    void openImport(bool isSysEx);
    void savePreset();
    void updateStatus(const juce::String&);
    void timerCallback() override;

    Processor& processor;
    PanelLook look;
    juce::TooltipWindow tooltips;
    std::vector<FaderItem> faders;
    std::vector<SwitchItem> switches;
    std::vector<MenuItem> menus;
    juce::ComboBox programs;
    juce::TextButton previous { "<" }, next { ">" };
    juce::TextButton loadSx { "IMPORT SYX" }, loadPreset { "LOAD PATCH" }, savePatch { "SAVE PATCH" };
    juce::Label status;
    juce::MidiKeyboardComponent keyboard;
    PerformanceWheel pitchWheel { true }, modWheel { false };
    std::unique_ptr<juce::FileChooser> chooser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
} // namespace aurora
