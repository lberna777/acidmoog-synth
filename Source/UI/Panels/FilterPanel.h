#pragma once

#include "../Components/KnobComponent.h"
#include "../Components/ToggleSwitchComponent.h"
#include "../Components/LedComponent.h"

class AcidMoogAudioProcessor;

/** Sezione filtro: il cuore del plugin. Cutoff e Resonance sono i knob piu' grandi
    del pannello, Drive ed Env>Filter secondari. */
class FilterPanel : public juce::Component,
                    private juce::Timer
{
public:
    explicit FilterPanel (AcidMoogAudioProcessor& processor);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    AcidMoogAudioProcessor& audioProcessor;

    ToggleSwitchComponent typeSwitch;
    KnobComponent cutoffKnob;
    KnobComponent resonanceKnob;
    KnobComponent driveKnob;
    KnobComponent envAmountKnob;
    LedComponent selfOscLed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterPanel)
};
