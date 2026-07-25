#include "Voice.h"

#include <cmath>

namespace
{
    constexpr float smoothingSeconds = 0.02f;
    constexpr float switchFadeSeconds = 0.005f;
    constexpr float selfOscillationThreshold = 0.9f;
}

void Voice::prepare (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    oscillator.prepare (currentSampleRate);
    sampler.prepare (currentSampleRate);
    filter.prepare (currentSampleRate);
    envelope.prepare (currentSampleRate);
    glide.prepare (currentSampleRate);

    for (auto* s : { &smoothedCutoff, &smoothedResonance, &smoothedDrive, &smoothedGain })
        s->reset (currentSampleRate, smoothingSeconds);

    switchFade.reset (currentSampleRate, switchFadeSeconds);
    switchFade.setCurrentAndTargetValue (1.0f);

    smoothedCutoff.setCurrentAndTargetValue (params.cutoffHz);
    smoothedResonance.setCurrentAndTargetValue (params.resonance);
    smoothedDrive.setCurrentAndTargetValue (params.drive);
    smoothedGain.setCurrentAndTargetValue (params.masterGain);
}

void Voice::reset()
{
    filter.reset();
    envelope.reset();
    switchFade.setCurrentAndTargetValue (1.0f);
    switchPending = false;
    selfOscillating = false;
    accentActive = false;
}

void Voice::setParameters (const Parameters& p)
{
    // Sorgente e tipo filtro non cambiano subito: si programmano e vengono applicati
    // dopo il fade a zero, altrimenti il salto nel path DSP produce un click.
    if (p.oscType != pendingOscType || p.filterType != pendingFilterType)
    {
        pendingOscType = p.oscType;
        pendingFilterType = p.filterType;

        if (p.oscType != params.oscType || p.filterType != params.filterType)
        {
            switchPending = true;
            switchFade.setTargetValue (0.0f);
        }
    }

    const auto previousOscType = params.oscType;
    const auto previousFilterType = params.filterType;

    params = p;

    // I due campi "in transizione" restano quelli vecchi finche' il fade non completa.
    params.oscType = previousOscType;
    params.filterType = previousFilterType;

    smoothedCutoff.setTargetValue (p.cutoffHz);
    smoothedResonance.setTargetValue (p.resonance);
    smoothedDrive.setTargetValue (p.drive);
    smoothedGain.setTargetValue (p.masterGain);

    envelope.setParameters (p.attack, p.decay, p.sustain, p.release);
    glide.setGlideTime (p.glideTime);
    glide.setMode (p.glideMode);
    accent.setAmount (p.accentAmount);
    accent.setThreshold (p.accentThreshold);
    sampler.setRootNote (p.sampleRootNote);
    filter.setOversamplingFactor (p.oversamplingFactor);
}

void Voice::applyPendingSourceSwitch()
{
    params.oscType = pendingOscType;
    params.filterType = pendingFilterType;
    filter.setType (params.filterType);

    // Il ladder porta stato: cambiando modo va azzerato, o il residuo del modo
    // precedente si sente come transiente.
    filter.reset();

    switchPending = false;
    switchFade.setTargetValue (1.0f);
}

void Voice::handleAction (const MonoVoiceManager::Action& action)
{
    using Type = MonoVoiceManager::Action::Type;

    switch (action.type)
    {
        case Type::Start:
        {
            currentVelocity = action.velocity;
            accentActive = accent.isAccented (action.velocity);
            accentGain = accent.getGainMultiplier (action.velocity);
            accentCutoffMultiplier = accent.getCutoffMultiplier (action.velocity);

            if (! envelope.isActive())
                glide.reset ((float) action.note);
            else
                glide.setTargetNote ((float) action.note, action.legato);

            if (action.retrigger)
            {
                oscillator.noteOn();
                sampler.noteOn();
                glide.reset ((float) action.note);
                envelope.noteOn();
            }
            else if (! envelope.isActive())
            {
                oscillator.noteOn();
                sampler.noteOn();
                envelope.noteOn();
            }

            break;
        }

        case Type::Release:
            envelope.noteOff();
            accentActive = false;
            break;

        case Type::None:
        default:
            break;
    }
}

void Voice::setPitchBend (float normalisedBend) noexcept
{
    pitchBendSemitones = normalisedBend * (float) params.pitchBendRange;
}

float Voice::renderSample() noexcept
{
    const auto envValue = envelope.getNextValue();

    const auto noteNumber = glide.getNextNote() + pitchBendSemitones;
    const auto frequency = 440.0f * std::pow (2.0f, (noteNumber - 69.0f) / 12.0f);

    const auto source = params.oscType == OscType::Sampler
                      ? sampler.getNextSample (frequency)
                      : oscillator.getNextSample (frequency);

    const auto baseCutoff = smoothedCutoff.getNextValue();
    const auto resonance = smoothedResonance.getNextValue();
    const auto drive = smoothedDrive.getNextValue();

    // L'envelope modula il cutoff in ottave: e' la modulazione esponenziale che
    // suona "giusta" all'orecchio, e con amount negativo il filtro si chiude.
    const auto envOctaves = params.envToFilterAmount * envValue * 4.0f;
    const auto cutoff = baseCutoff * std::pow (2.0f, envOctaves) * accentCutoffMultiplier;

    const auto filtered = filter.processSample (source, cutoff, resonance, drive);

    selfOscillating = resonance >= selfOscillationThreshold;

    const auto fade = switchFade.getNextValue();

    if (switchPending && fade <= 0.0001f)
        applyPendingSourceSwitch();

    return filtered * envValue * accentGain * smoothedGain.getNextValue() * fade;
}

void Voice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (! envelope.isActive())
    {
        accentActive = false;
        return;
    }

    auto* left = buffer.getWritePointer (0, startSample);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1, startSample) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto value = renderSample();

        left[i] += value;

        if (right != nullptr)
            right[i] += value;
    }
}
