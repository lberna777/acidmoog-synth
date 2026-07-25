#include "Oscillator.h"

#include <cmath>

void Oscillator::prepare (double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    phase = 0.0f;
}

void Oscillator::noteOn()
{
    phase = 0.0f;
}

float Oscillator::polyBlep (float t, float phaseIncrement)
{
    if (phaseIncrement <= 0.0f)
        return 0.0f;

    if (t < phaseIncrement)                 // subito dopo il salto
    {
        t /= phaseIncrement;
        return t + t - t * t - 1.0f;
    }

    if (t > 1.0f - phaseIncrement)          // subito prima del salto
    {
        t = (t - 1.0f) / phaseIncrement;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
}

float Oscillator::getNextSample (float frequencyHz)
{
    const auto phaseIncrement = static_cast<float> (frequencyHz / currentSampleRate);

    // Saw naive nell'intervallo [-1, 1], poi corretta attorno alla discontinuita'.
    float value = 2.0f * phase - 1.0f;
    value -= polyBlep (phase, phaseIncrement);

    phase += phaseIncrement;
    while (phase >= 1.0f)
        phase -= 1.0f;

    return value;
}
