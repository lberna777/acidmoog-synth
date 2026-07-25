#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/** Selettore a levetta a N posizioni, agganciato a un parametro choice.

    Sostituisce la ComboBox per i parametri discreti: cliccando si avanza alla
    posizione successiva, come su un vero selettore da pannello.
*/
class ToggleSwitchComponent : public juce::Component,
                              private juce::Value::Listener
{
public:
    ToggleSwitchComponent (juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& parameterID,
                           const juce::String& labelText,
                           juce::StringArray positionNames);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void valueChanged (juce::Value&) override { repaint(); }

    int getCurrentIndex() const;

    juce::AudioProcessorValueTreeState& state;
    juce::String paramID;
    juce::String title;
    juce::StringArray positions;
    juce::Value paramValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleSwitchComponent)
};
