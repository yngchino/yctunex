#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace ParamID {
inline constexpr const char* inputGain  = "inputGain";
inline constexpr const char* outputGain = "outputGain";
inline constexpr const char* mix        = "mix";
}

class YCTuneXAudioProcessor : public juce::AudioProcessor
{
public:
    YCTuneXAudioProcessor();
    ~YCTuneXAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "YC TUNE X"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inputLevel { 0.0f }, outputLevel { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::SmoothedValue<float> inGainSm, outGainSm, mixSm;
    juce::AudioBuffer<float> dryBuffer;
    std::atomic<float>* pIn = nullptr;
    std::atomic<float>* pOut = nullptr;
    std::atomic<float>* pMix = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YCTuneXAudioProcessor)
};
