#pragma once

#include "Oscillator.h"
#include "SamplerSource.h"
#include "LadderFilter.h"
#include "Envelope.h"
#include "GlideProcessor.h"
#include "AccentProcessor.h"
#include "../Voicing/MonoVoiceManager.h"

/** La voce mono: mette in catena sorgente -> drive/filtro -> ampiezza.

    Riceve i valori dei parametri una volta per blocco (struct Parameters) e li
    lascia scorrere attraverso SmoothedValue, cosi' automazioni e movimenti di knob
    non producono zipper noise.
*/
class Voice
{
public:
    struct Parameters
    {
        OscType oscType { OscType::Synth };
        FilterType filterType { FilterType::LowPass };
        float cutoffHz { 1000.0f };
        float resonance { 0.2f };       // 0..1
        float drive { 0.0f };           // 0..1
        float attack { 0.001f };
        float decay { 0.15f };
        float sustain { 0.7f };
        float release { 0.1f };
        float envToFilterAmount { 0.0f }; // -1..1
        float accentAmount { 50.0f };     // percentuale
        int accentThreshold { 100 };
        float glideTime { 0.08f };
        GlideMode glideMode { GlideMode::LegatoOnly };
        int pitchBendRange { 2 };
        float masterGain { 1.0f };        // lineare
        int sampleRootNote { 60 };
        int oversamplingFactor { 2 };
    };

    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    void setParameters (const Parameters& p);

    void handleAction (const MonoVoiceManager::Action& action);
    void setPitchBend (float normalisedBend) noexcept;   // -1..1

    /** Riempie il buffer mono aggiungendo l'uscita della voce. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    bool isActive() const noexcept { return envelope.isActive(); }

    SamplerSource& getSampler() noexcept { return sampler; }

    /** True quando la risonanza e' oltre la soglia di autooscillazione (per il LED). */
    bool isSelfOscillating() const noexcept { return selfOscillating; }
    bool isAccentActive() const noexcept    { return accentActive; }

private:
    float renderSample() noexcept;
    void applyPendingSourceSwitch();

    Oscillator oscillator;
    SamplerSource sampler;
    LadderFilter filter;
    Envelope envelope;
    GlideProcessor glide;
    AccentProcessor accent;

    Parameters params;

    juce::SmoothedValue<float> smoothedCutoff;
    juce::SmoothedValue<float> smoothedResonance;
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedGain;

    // Fade usato per rendere click-free il cambio di sorgente o di tipo filtro:
    // si scende a zero, si applica il cambio, si risale.
    juce::SmoothedValue<float> switchFade;
    OscType pendingOscType { OscType::Synth };
    FilterType pendingFilterType { FilterType::LowPass };
    bool switchPending { false };

    double currentSampleRate { 44100.0 };
    int currentVelocity { 100 };
    float pitchBendSemitones { 0.0f };
    float accentGain { 1.0f };
    float accentCutoffMultiplier { 1.0f };
    bool selfOscillating { false };
    bool accentActive { false };
};
