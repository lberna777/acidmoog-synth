#include "Envelope.h"

void Envelope::prepare (double sampleRate)
{
    adsr.setSampleRate (sampleRate > 0.0 ? sampleRate : 44100.0);
    adsr.setParameters (params);
    adsr.reset();
}

void Envelope::setParameters (float attackSeconds, float decaySeconds,
                              float sustainLevel, float releaseSeconds)
{
    // Un attack esattamente a zero produce un click sul primo campione: il minimo
    // di 1 ms resta percettivamente istantaneo ma smussa il fronte.
    params.attack  = juce::jmax (0.001f, attackSeconds);
    params.decay   = juce::jmax (0.001f, decaySeconds);
    params.sustain = juce::jlimit (0.0f, 1.0f, sustainLevel);
    params.release = juce::jmax (0.005f, releaseSeconds);

    adsr.setParameters (params);
}

void Envelope::noteOn()  { adsr.noteOn(); }
void Envelope::noteOff() { adsr.noteOff(); }
void Envelope::reset()   { adsr.reset(); }

float Envelope::getNextValue() noexcept
{
    return adsr.getNextSample();
}
