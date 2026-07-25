#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/** Traduce la velocity in "accento" alla maniera del TB-303: le note oltre soglia
    suonano piu' forti e piu' aperte in frequenza, non solo piu' forti. */
class AccentProcessor
{
public:
    void setAmount (float percent) noexcept    { amount = juce::jlimit (0.0f, 1.0f, percent * 0.01f); }
    void setThreshold (int velocity) noexcept  { threshold = juce::jlimit (1, 127, velocity); }

    bool isAccented (int velocity) const noexcept { return velocity >= threshold; }

    /** Moltiplicatore di ampiezza per la nota corrente (1.0 = nessun accento). */
    float getGainMultiplier (int velocity) const noexcept;

    /** Moltiplicatore da applicare al cutoff: l'accento apre il filtro, che e' cio'
        che rende l'accento del 303 riconoscibile anche a volume pareggiato. */
    float getCutoffMultiplier (int velocity) const noexcept;

private:
    /** Quanto la velocity supera la soglia, normalizzato 0..1. */
    float accentStrength (int velocity) const noexcept;

    float amount { 0.5f };
    int threshold { 100 };
};
