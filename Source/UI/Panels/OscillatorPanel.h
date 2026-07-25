#pragma once

#include "../Components/KnobComponent.h"
#include "../Components/ToggleSwitchComponent.h"
#include "../Components/WaveformDisplay.h"
#include "../Components/FileDropZone.h"

class AcidMoogAudioProcessor;

/** Sezione sorgente: switch Synth/Sampler e, in modalita' Sampler, drop zone,
    root note e display della forma d'onda. */
class OscillatorPanel : public juce::Component,
                        private juce::Timer
{
public:
    explicit OscillatorPanel (AcidMoogAudioProcessor& processor);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateModeVisibility();
    void loadFile (const juce::File& file);

    AcidMoogAudioProcessor& audioProcessor;

    ToggleSwitchComponent sourceSwitch;
    KnobComponent rootNoteKnob;
    WaveformDisplay waveform;
    FileDropZone dropZone;

    bool samplerMode { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorPanel)
};
