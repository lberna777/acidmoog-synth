#include "GlideProcessor.h"

#include <cmath>

void GlideProcessor::prepare (double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    incrementPerSample = 0.0f;
}

void GlideProcessor::reset (float noteNumber)
{
    currentNote = noteNumber;
    targetNote = noteNumber;
    incrementPerSample = 0.0f;
}

void GlideProcessor::setTargetNote (float noteNumber, bool isLegato) noexcept
{
    targetNote = noteNumber;

    const auto glideApplies = glideSeconds > 0.0f
                           && (mode == GlideMode::Always || isLegato);

    if (! glideApplies)
    {
        currentNote = noteNumber;
        incrementPerSample = 0.0f;
        return;
    }

    const auto distance = targetNote - currentNote;
    const auto numSamples = glideSeconds * (float) currentSampleRate;

    incrementPerSample = numSamples > 0.0f ? distance / numSamples : 0.0f;
}

float GlideProcessor::getNextNote() noexcept
{
    if (incrementPerSample == 0.0f)
        return currentNote;

    currentNote += incrementPerSample;

    // Arrivati (o superati) il target, agganciamo esattamente e fermiamo la rampa.
    const auto arrived = incrementPerSample > 0.0f ? currentNote >= targetNote
                                                   : currentNote <= targetNote;

    if (arrived)
    {
        currentNote = targetNote;
        incrementPerSample = 0.0f;
    }

    return currentNote;
}
