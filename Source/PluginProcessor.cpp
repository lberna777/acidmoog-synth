#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    float getFloat (juce::AudioProcessorValueTreeState& apvts, const char* id)
    {
        return apvts.getRawParameterValue (id)->load();
    }

    int getChoice (juce::AudioProcessorValueTreeState& apvts, const char* id)
    {
        return (int) apvts.getRawParameterValue (id)->load();
    }
}

AcidMoogAudioProcessor::AcidMoogAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

bool AcidMoogAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void AcidMoogAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voice.prepare (sampleRate, samplesPerBlock);
    voice.reset();
    voiceManager.allNotesOff();
    updateVoiceParameters();
}

void AcidMoogAudioProcessor::updateVoiceParameters()
{
    Voice::Parameters p;

    p.oscType    = (OscType) getChoice (apvts, ParamID::oscType);
    p.filterType = (FilterType) getChoice (apvts, ParamID::filterType);
    p.cutoffHz   = getFloat (apvts, ParamID::cutoff);
    p.resonance  = getFloat (apvts, ParamID::resonance) * 0.01f;
    p.drive      = getFloat (apvts, ParamID::drive) * 0.01f;
    p.attack     = getFloat (apvts, ParamID::attack);
    p.decay      = getFloat (apvts, ParamID::decay);
    p.sustain    = getFloat (apvts, ParamID::sustain) * 0.01f;
    p.release    = getFloat (apvts, ParamID::release);
    p.envToFilterAmount = getFloat (apvts, ParamID::envToFilterAmount) * 0.01f;
    p.accentAmount    = getFloat (apvts, ParamID::accentAmount);
    p.accentThreshold = (int) getFloat (apvts, ParamID::accentThreshold);
    p.glideTime  = getFloat (apvts, ParamID::glideTime);
    p.glideMode  = (GlideMode) getChoice (apvts, ParamID::glideMode);
    p.pitchBendRange = (int) getFloat (apvts, ParamID::pitchBendRange);
    p.masterGain = juce::Decibels::decibelsToGain (getFloat (apvts, ParamID::masterVolume), -60.0f);
    p.sampleRootNote = (int) getFloat (apvts, ParamID::sampleRootNote);

    const int factors[] = { 1, 2, 4 };
    p.oversamplingFactor = factors[juce::jlimit (0, 2, getChoice (apvts, ParamID::oversamplingQuality))];

    voiceManager.setLegatoMode ((LegatoMode) getChoice (apvts, ParamID::legatoMode));
    voice.setParameters (p);
}

void AcidMoogAudioProcessor::handleMidiEvent (const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        voice.handleAction (voiceManager.noteOn (message.getNoteNumber(),
                                                 message.getVelocity()));
    }
    else if (message.isNoteOff())
    {
        voice.handleAction (voiceManager.noteOff (message.getNoteNumber()));
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        voiceManager.allNotesOff();
        voice.handleAction ({ MonoVoiceManager::Action::Type::Release });
    }
    else if (message.isPitchWheel())
    {
        // 0..16383 con centro a 8192.
        const auto normalised = (message.getPitchWheelValue() - 8192) / 8192.0f;
        voice.setPitchBend (normalised);
    }
}

void AcidMoogAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
    updateVoiceParameters();

    int lastEventSample = 0;

    // Rendering a sotto-blocchi delimitati dagli eventi MIDI: cosi' le note partono
    // esattamente sul campione indicato dall'host, non all'inizio del blocco.
    for (const auto metadata : midiMessages)
    {
        const auto eventSample = juce::jlimit (0, buffer.getNumSamples(), metadata.samplePosition);
        const auto numSamples = eventSample - lastEventSample;

        if (numSamples > 0)
        {
            voice.renderNextBlock (buffer, lastEventSample, numSamples);
            lastEventSample = eventSample;
        }

        handleMidiEvent (metadata.getMessage());
    }

    const auto remaining = buffer.getNumSamples() - lastEventSample;

    if (remaining > 0)
        voice.renderNextBlock (buffer, lastEventSample, remaining);
}

bool AcidMoogAudioProcessor::loadSampleFile (const juce::File& file)
{
    return voice.getSampler().loadFile (file);
}

juce::AudioProcessorEditor* AcidMoogAudioProcessor::createEditor()
{
    return new AcidMoogAudioProcessorEditor (*this);
}

void AcidMoogAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AcidMoogAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidMoogAudioProcessor();
}
