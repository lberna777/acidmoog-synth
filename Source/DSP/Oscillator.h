#pragma once

#include "SoundSource.h"

/** Oscillatore a dente di sega band-limited via PolyBLEP.

    La saw naive ha un salto di ampiezza per ciclo, che in digitale genera aliasing
    su tutta la banda. PolyBLEP corregge i campioni immediatamente attorno al salto,
    abbattendo l'aliasing con un costo trascurabile rispetto a wavetable/BLIT.
*/
class Oscillator : public SoundSource
{
public:
    void prepare (double sampleRate) override;
    void noteOn() override;
    float getNextSample (float frequencyHz) override;

private:
    /** Correzione PolyBLEP attorno alla discontinuita', t = fase normalizzata. */
    static float polyBlep (float t, float phaseIncrement);

    double currentSampleRate { 44100.0 };
    float phase { 0.0f };   // 0..1
};
