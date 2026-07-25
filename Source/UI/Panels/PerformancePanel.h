#pragma once

#include "../Components/KnobComponent.h"
#include "../Components/ToggleSwitchComponent.h"
#include "../Components/LedComponent.h"

class AcidMoogAudioProcessor;

/** Sezione performance: accento, glide, pitch bend e volume. */
class PerformancePanel : public juce::Component,
                         private juce::Timer
{
public:
    explicit PerformancePanel (AcidMoogAudioProcessor& processor);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    AcidMoogAudioProcessor& audioProcessor;

    KnobComponent accentAmountKnob;
    KnobComponent accentThresholdKnob;
    KnobComponent glideTimeKnob;
    KnobComponent pitchBendKnob;
    KnobComponent volumeKnob;
    ToggleSwitchComponent glideModeSwitch;
    ToggleSwitchComponent legatoModeSwitch;
    LedComponent accentLed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformancePanel)
};
