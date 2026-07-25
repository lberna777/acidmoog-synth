#pragma once

#include "../Components/KnobComponent.h"

class AcidMoogAudioProcessor;

/** Sezione inviluppo: quattro knob A/D/S/R allineati. */
class EnvelopePanel : public juce::Component
{
public:
    explicit EnvelopePanel (AcidMoogAudioProcessor& processor);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    KnobComponent attackKnob;
    KnobComponent decayKnob;
    KnobComponent sustainKnob;
    KnobComponent releaseKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopePanel)
};
