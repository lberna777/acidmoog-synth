#include "ParameterLayout.h"

using APVTS = juce::AudioProcessorValueTreeState;

namespace
{
    // Skew centrato: rende la corsa del knob percettivamente uniforme sui tempi
    // brevi, dove si concentra l'uso musicale di attack/decay/release.
    juce::NormalisableRange<float> timeRange (float maxSeconds, float centreSeconds)
    {
        juce::NormalisableRange<float> r { 0.0f, maxSeconds };
        r.setSkewForCentre (centreSeconds);
        return r;
    }

    std::unique_ptr<juce::AudioParameterFloat> percent (const char* id, const juce::String& name, float def)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> { 0.0f, 100.0f }, def);
    }
}

APVTS::ParameterLayout createParameterLayout()
{
    APVTS::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::oscType, 1 }, "Source",
        juce::StringArray { "Synth", "Sampler" }, 0));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::filterType, 1 }, "Filter Type",
        juce::StringArray { "LP", "HP", "BP" }, 0));

    {
        juce::NormalisableRange<float> cutoffRange { 20.0f, 20000.0f };
        cutoffRange.setSkewForCentre (1000.0f);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamID::cutoff, 1 }, "Cutoff", cutoffRange, 1000.0f));
    }

    layout.add (percent (ParamID::resonance, "Resonance", 20.0f));
    layout.add (percent (ParamID::drive, "Drive", 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::attack, 1 }, "Attack", timeRange (5.0f, 0.2f), 0.001f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::decay, 1 }, "Decay", timeRange (5.0f, 0.2f), 0.150f));
    layout.add (percent (ParamID::sustain, "Sustain", 70.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::release, 1 }, "Release", timeRange (5.0f, 0.2f), 0.100f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::envToFilterAmount, 1 }, "Env > Filter",
        juce::NormalisableRange<float> { -100.0f, 100.0f }, 0.0f));

    layout.add (percent (ParamID::accentAmount, "Accent Amount", 50.0f));
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamID::accentThreshold, 1 }, "Accent Threshold", 1, 127, 100));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::glideTime, 1 }, "Glide Time", timeRange (2.0f, 0.15f), 0.080f));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::glideMode, 1 }, "Glide Mode",
        juce::StringArray { "Always", "Legato Only" }, 1));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::legatoMode, 1 }, "Legato Mode",
        juce::StringArray { "Auto", "Always Retrigger" }, 0));

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamID::pitchBendRange, 1 }, "Pitch Bend Range", 1, 24, 2));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::masterVolume, 1 }, "Master Volume",
        juce::NormalisableRange<float> { -60.0f, 6.0f }, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamID::sampleRootNote, 1 }, "Sample Root Note", 0, 127, 60));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::oversamplingQuality, 1 }, "Oversampling",
        juce::StringArray { "1x", "2x", "4x" }, 1));

    return layout;
}
