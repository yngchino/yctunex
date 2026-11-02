#pragma once
#include "PluginProcessor.h"

class YCTuneXAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit YCTuneXAudioProcessorEditor (YCTuneXAudioProcessor&);
    ~YCTuneXAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint (meterArea); }

    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void setupKnob (Knob&, const juce::String& text, const char* paramID);

    YCTuneXAudioProcessor& proc;
    Knob inKnob, mixKnob, outKnob;
    juce::Rectangle<int> meterArea;
    float inShown = 0.0f, outShown = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YCTuneXAudioProcessorEditor)
};
