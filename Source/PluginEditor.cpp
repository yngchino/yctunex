#include "PluginEditor.h"

static const juce::Colour kBg (0xff0b0b0d), kPanel (0xff1c1c20), kRed (0xffe3202b);

YCTuneXAudioProcessorEditor::YCTuneXAudioProcessorEditor (YCTuneXAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setupKnob (inKnob,  "INPUT",    ParamID::inputGain);
    setupKnob (mixKnob, "DRY/WET",  ParamID::mix);
    setupKnob (outKnob, "OUTPUT",   ParamID::outputGain);
    setSize (560, 300);
    startTimerHz (30);
}

void YCTuneXAudioProcessorEditor::setupKnob (Knob& k, const juce::String& text, const char* id)
{
    k.slider.setColour (juce::Slider::rotarySliderFillColourId, kRed);
    k.slider.setColour (juce::Slider::thumbColourId, kRed);
    k.slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3a3a40));
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (k.slider);
    addAndMakeVisible (k.label);
    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.slider);
}

void YCTuneXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);
    g.setColour (kPanel);
    g.fillRoundedRectangle (getLocalBounds().reduced (8).toFloat(), 10.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("YC TUNE X", 20, 14, 300, 26, juce::Justification::left);
    g.setColour (kRed);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ULTRA ROBOTIC PITCH ENGINE  -  FASE 2 (bypass con ganancias)", 20, 40, 460, 16, juce::Justification::left);

    inShown  = juce::jmax (proc.inputLevel.load(),  inShown  * 0.85f);
    outShown = juce::jmax (proc.outputLevel.load(), outShown * 0.85f);
    auto drawMeter = [&] (juce::Rectangle<int> r, float lvl)
    {
        g.setColour (juce::Colour (0xff2a2a2f)); g.fillRect (r);
        const float db = juce::Decibels::gainToDecibels (lvl, -60.0f);
        const float h = juce::jmap (db, -60.0f, 0.0f, 0.0f, (float) r.getHeight());
        g.setColour (kRed); g.fillRect (r.removeFromBottom ((int) h));
    };
    drawMeter (meterArea.withWidth (10), inShown);
    drawMeter (meterArea.withX (meterArea.getX() + 14).withWidth (10), outShown);
}

void YCTuneXAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20).withTrimmedTop (60).withTrimmedRight (40);
    const int w = area.getWidth() / 3;
    for (auto* k : { &inKnob, &mixKnob, &outKnob })
    {
        auto col = area.removeFromLeft (w);
        k->label.setBounds (col.removeFromTop (20));
        k->slider.setBounds (col);
    }
    meterArea = getLocalBounds().withLeft (getWidth() - 44).withTrimmedTop (70).withTrimmedBottom (20).withWidth (24);
}
