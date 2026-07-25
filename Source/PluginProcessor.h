#pragma once

#include "Params/ParameterLayout.h"
#include "DSP/Voice.h"
#include "Voicing/MonoVoiceManager.h"

class AcidMoogAudioProcessor : public juce::AudioProcessor
{
public:
    AcidMoogAudioProcessor();
    ~AcidMoogAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    Voice& getVoice() noexcept { return voice; }

    /** Caricamento sample dal drop zone della UI (message thread). */
    bool loadSampleFile (const juce::File& file);

    juce::String getLoadedSampleName() { return voice.getSampler().getSampleName(); }

private:
    void updateVoiceParameters();
    void handleMidiEvent (const juce::MidiMessage& message);

    juce::AudioProcessorValueTreeState apvts;
    MonoVoiceManager voiceManager;
    Voice voice;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidMoogAudioProcessor)
};
