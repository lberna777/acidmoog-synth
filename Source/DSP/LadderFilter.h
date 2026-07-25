#pragma once

#include "../Params/ParamTypes.h"

/** Filtro ladder 4 poli in stile Moog/303, con drive pre-filtro e oversampling interno.

    La non linearita' (tanh su ogni stadio + drive in ingresso) e' quello che da' il
    carattere "acid" quando risonanza e drive sono spinti, ma genera armoniche sopra
    Nyquist: per questo il filtro gira internamente a 1x/2x/4x la frequenza di
    campionamento e decima in uscita.

    Le uscite LP/HP/BP sono derivate dagli stadi del ladder:
      LP = uscita del quarto polo
      HP = ingresso - LP (complemento)
      BP = differenza fra secondo e quarto polo
*/
class LadderFilter
{
public:
    void prepare (double sampleRate);
    void reset();

    void setOversamplingFactor (int factor) noexcept;   // 1, 2 o 4
    void setType (FilterType t) noexcept                { type = t; }

    /** cutoffHz gia' modulato da envelope/accent, resonance e drive in 0..1. */
    float processSample (float input, float cutoffHz, float resonance, float drive) noexcept;

private:
    float processOneStep (float input, float g, float resonance) noexcept;

    static float fastTanh (float x) noexcept;

    double baseSampleRate { 44100.0 };
    double oversampledRate { 44100.0 };
    int oversamplingFactor { 2 };

    float stage[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    float delay[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    float lastStage4 { 0.0f };

    FilterType type { FilterType::LowPass };
};
