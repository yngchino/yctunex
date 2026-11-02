#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout YCTuneXAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::inputGain, 1 }, "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::outputGain, 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::mix, 1 }, "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));
    return { p.begin(), p.end() };
}

YCTuneXAudioProcessor::YCTuneXAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pIn  = apvts.getRawParameterValue (ParamID::inputGain);
    pOut = apvts.getRawParameterValue (ParamID::outputGain);
    pMix = apvts.getRawParameterValue (ParamID::mix);
}

bool YCTuneXAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void YCTuneXAudioProcessor::prepareToPlay (double sr, int maxBlock)
{
    const double ramp = 0.02;
    inGainSm.reset (sr, ramp);  inGainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outGainSm.reset (sr, ramp); outGainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    mixSm.reset (sr, ramp);     mixSm.setCurrentAndTargetValue (pMix->load() * 0.01f);
    dryBuffer.setSize (2, juce::jmax (1, maxBlock), false, true, false);   // única reserva: fuera del hilo de audio
    setLatencySamples (0);   // Fase 2: sin latencia. Se actualizará con el motor DSP.
}

void YCTuneXAudioProcessor::processBlock (juce::AudioBuffer<float>& buf, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nCh = juce::jmin (buf.getNumChannels(), 2);
    const int n = buf.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < buf.getNumChannels(); ++c) buf.clear (c, 0, n);
    if (n > dryBuffer.getNumSamples()) return;   // host excede maxBlock: no procesar en vez de reservar memoria

    inGainSm.setTargetValue  (juce::Decibels::decibelsToGain (pIn->load()));
    outGainSm.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    mixSm.setTargetValue     (pMix->load() * 0.01f);

    float inPeak = 0.0f, outPeak = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        const float g = inGainSm.getNextValue(), og = outGainSm.getNextValue(), m = mixSm.getNextValue();
        for (int c = 0; c < nCh; ++c)
        {
            float x = buf.getSample (c, i);
            if (! std::isfinite (x)) x = 0.0f;           // protección NaN/Inf
            const float dry = x * g;
            const float wet = dry;                        // Fase 3+: aquí entra el motor de pitch
            float y = (dry * (1.0f - m) + wet * m) * og;
            y = juce::jlimit (-1.0f, 1.0f, y);            // protección anti-clipping
            inPeak  = juce::jmax (inPeak,  std::abs (dry));
            outPeak = juce::jmax (outPeak, std::abs (y));
            buf.setSample (c, i, y);
        }
    }
    inputLevel.store  (inPeak);
    outputLevel.store (outPeak);
}

void YCTuneXAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void YCTuneXAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* YCTuneXAudioProcessor::createEditor() { return new YCTuneXAudioProcessorEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new YCTuneXAudioProcessor(); }
