#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/** LED da pannello con alone: acceso/spento impostato dal chiamante.

    L'accensione non e' istantanea ma segue una breve rampa, come un LED reale:
    evita lo sfarfallio quando lo stato oscilla fra un frame e l'altro.
*/
class LedComponent : public juce::Component,
                     private juce::Timer
{
public:
    LedComponent (juce::Colour colour, const juce::String& labelText);

    void setState (bool shouldBeOn) noexcept { target = shouldBeOn ? 1.0f : 0.0f; }

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    juce::Colour ledColour;
    juce::String title;
    float brightness { 0.0f };
    float target { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LedComponent)
};
