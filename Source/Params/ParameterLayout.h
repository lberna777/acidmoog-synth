#pragma once

#include "ParamTypes.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    inline constexpr const char* oscType            = "oscType";
    inline constexpr const char* filterType         = "filterType";
    inline constexpr const char* cutoff             = "cutoff";
    inline constexpr const char* resonance          = "resonance";
    inline constexpr const char* drive              = "drive";
    inline constexpr const char* attack             = "attack";
    inline constexpr const char* decay              = "decay";
    inline constexpr const char* sustain            = "sustain";
    inline constexpr const char* release            = "release";
    inline constexpr const char* envToFilterAmount  = "envToFilterAmount";
    inline constexpr const char* accentAmount       = "accentAmount";
    inline constexpr const char* accentThreshold    = "accentThreshold";
    inline constexpr const char* glideTime          = "glideTime";
    inline constexpr const char* glideMode          = "glideMode";
    inline constexpr const char* legatoMode         = "legatoMode";
    inline constexpr const char* pitchBendRange     = "pitchBendRange";
    inline constexpr const char* masterVolume       = "masterVolume";
    inline constexpr const char* sampleRootNote     = "sampleRootNote";
    inline constexpr const char* oversamplingQuality = "oversamplingQuality";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
