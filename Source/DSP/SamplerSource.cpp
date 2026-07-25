#include "SamplerSource.h"

#include <cmath>

SamplerSource::SamplerSource()
{
    formatManager.registerBasicFormats();
}

void SamplerSource::prepare (double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    readPosition = 0.0;
    finished = true;
}

void SamplerSource::noteOn()
{
    readPosition = 0.0;
    finished = ! sampleLoaded.load();
}

bool SamplerSource::loadFile (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;

    juce::AudioBuffer<float> newBuffer ((int) reader->numChannels,
                                        (int) reader->lengthInSamples);
    reader->read (&newBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

    // Il thread audio legge solo quando sampleLoaded e' true: azzeriamo il flag,
    // scambiamo i dati e lo rialziamo.
    sampleLoaded.store (false);
    buffer = std::move (newBuffer);
    sampleFileRate = reader->sampleRate > 0.0 ? reader->sampleRate : currentSampleRate;
    loadedName = file.getFileName();
    readPosition = 0.0;
    finished = true;
    sampleLoaded.store (true);

    return true;
}

float SamplerSource::getNextSample (float frequencyHz)
{
    if (! sampleLoaded.load() || finished)
        return 0.0f;

    const auto numSamples = buffer.getNumSamples();

    if (numSamples < 4)
        return 0.0f;

    // Rapporto di lettura: pitch relativo alla root note, piu' compensazione
    // dell'eventuale differenza fra sample rate del file e quello dell'host.
    const auto rootFreq = 440.0 * std::pow (2.0, (rootNote.load() - 69) / 12.0);
    const auto ratio = (frequencyHz / rootFreq) * (sampleFileRate / currentSampleRate);

    const auto index = (int) readPosition;

    if (index >= numSamples - 2)
    {
        finished = true;
        return 0.0f;
    }

    const auto frac = (float) (readPosition - (double) index);

    // Catmull-Rom su 4 punti; ai bordi ripiega sul campione piu' vicino disponibile.
    auto sampleAt = [this, numSamples] (int channel, int i) -> float
    {
        i = juce::jlimit (0, numSamples - 1, i);
        return buffer.getSample (channel, i);
    };

    float mixed = 0.0f;
    const auto numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto p0 = sampleAt (ch, index - 1);
        const auto p1 = sampleAt (ch, index);
        const auto p2 = sampleAt (ch, index + 1);
        const auto p3 = sampleAt (ch, index + 2);

        const auto a = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);
        const auto b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
        const auto c = 0.5f * (-p0 + p2);

        mixed += ((a * frac + b) * frac + c) * frac + p1;
    }

    if (numChannels > 1)
        mixed /= (float) numChannels;

    readPosition += ratio;

    return mixed;
}
