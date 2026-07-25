#include "LadderFilter.h"

#include <cmath>

void LadderFilter::prepare (double sampleRate)
{
    baseSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    oversampledRate = baseSampleRate * oversamplingFactor;
    reset();
}

void LadderFilter::reset()
{
    for (int i = 0; i < 4; ++i)
    {
        stage[i] = 0.0f;
        delay[i] = 0.0f;
    }

    lastStage4 = 0.0f;
}

void LadderFilter::setOversamplingFactor (int factor) noexcept
{
    factor = (factor == 1 || factor == 2 || factor == 4) ? factor : 2;

    if (factor != oversamplingFactor)
    {
        oversamplingFactor = factor;
        oversampledRate = baseSampleRate * oversamplingFactor;
        reset();
    }
}

float LadderFilter::fastTanh (float x) noexcept
{
    // Approssimazione razionale: monotona e limitata come tanh, senza il costo di std::tanh
    // per campione (qui chiamata 4 volte per stadio per ogni step di oversampling).
    const auto x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

float LadderFilter::processOneStep (float input, float g, float resonance) noexcept
{
    // Feedback dalla coda del ladder: e' cio' che produce la risonanza e, oltre
    // ~0.9, l'autooscillazione tipica del 303.
    const auto feedback = 4.0f * resonance * lastStage4;
    const auto in = fastTanh (input - feedback);

    stage[0] = stage[0] + g * (in - fastTanh (stage[0]));
    stage[1] = stage[1] + g * (fastTanh (stage[0]) - fastTanh (stage[1]));
    stage[2] = stage[2] + g * (fastTanh (stage[1]) - fastTanh (stage[2]));
    stage[3] = stage[3] + g * (fastTanh (stage[2]) - fastTanh (stage[3]));

    // Mezzo campione di ritardo sul feedback: stabilizza il loop ad alta risonanza.
    lastStage4 = 0.5f * (stage[3] + delay[3]);
    delay[3] = stage[3];

    switch (type)
    {
        case FilterType::HighPass:  return in - stage[3];
        case FilterType::BandPass:  return stage[1] - stage[3];
        case FilterType::LowPass:
        default:                    return stage[3];
    }
}

float LadderFilter::processSample (float input, float cutoffHz, float resonance, float drive) noexcept
{
    // Drive: 0..1 mappato su un guadagno pre-filtro fino a ~+24 dB. La saturazione
    // vera avviene dentro il ladder (fastTanh), qui si decide solo quanto spingerlo.
    const auto driveGain = 1.0f + drive * 15.0f;
    const auto driven = input * driveGain;

    // Compensazione: senza, alzare il drive alza anche il volume percepito.
    const auto makeUp = 1.0f / std::sqrt (driveGain);

    const auto nyquist = (float) (oversampledRate * 0.49);
    const auto fc = juce::jlimit (20.0f, nyquist, cutoffHz);
    const auto g = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi
                                    * fc / (float) oversampledRate);

    const auto res = juce::jlimit (0.0f, 1.0f, resonance);

    // Oversampling per zero-order hold: lo stesso campione viene processato N volte
    // e le uscite mediate. E' una decimazione grezza ma sufficiente a spostare
    // l'aliasing generato dalle non linearita' fuori dalla banda udibile.
    float sum = 0.0f;

    for (int i = 0; i < oversamplingFactor; ++i)
        sum += processOneStep (driven, g, res);

    return (sum / (float) oversamplingFactor) * makeUp;
}
