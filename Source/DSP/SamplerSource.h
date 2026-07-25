#pragma once

#include "SoundSource.h"

#include <juce_audio_formats/juce_audio_formats.h>

/** Sampler a singolo file, pitch-tracked su tutta la tastiera.

    Il sample viene riprodotto a velocita' variabile in base al rapporto fra la nota
    suonata e la root note. La lettura usa interpolazione cubica (Catmull-Rom): agli
    estremi della tastiera l'interpolazione lineare produce artefatti udibili.
*/
class SamplerSource : public SoundSource
{
public:
    SamplerSource();

    void prepare (double sampleRate) override;
    void noteOn() override;
    float getNextSample (float frequencyHz) override;

    /** Carica un file audio; thread-safe rispetto al thread audio (swap atomico). */
    bool loadFile (const juce::File& file);

    bool hasSample() const noexcept          { return sampleLoaded.load(); }
    juce::String getSampleName() const       { return loadedName; }

    /** Copia dei dati per la waveform display (chiamare dal message thread). */
    const juce::AudioBuffer<float>& getBuffer() const noexcept { return buffer; }

    void setRootNote (int midiNote) noexcept { rootNote = midiNote; }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> buffer;
    juce::String loadedName;

    std::atomic<bool> sampleLoaded { false };
    std::atomic<int> rootNote { 60 };

    double currentSampleRate { 44100.0 };
    double sampleFileRate { 44100.0 };
    double readPosition { 0.0 };
    bool finished { true };
};
