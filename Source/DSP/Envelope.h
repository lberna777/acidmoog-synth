#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/** ADSR sull'ampiezza, il cui valore corrente serve anche come sorgente di
    modulazione per il cutoff (envToFilterAmount). */
class Envelope
{
public:
    void prepare (double sampleRate);

    void setParameters (float attackSeconds, float decaySeconds,
                        float sustainLevel, float releaseSeconds);

    void noteOn();
    void noteOff();
    void reset();

    /** Avanza di un campione e restituisce il valore 0..1. */
    float getNextValue() noexcept;

    bool isActive() const noexcept { return adsr.isActive(); }

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters params;
};
