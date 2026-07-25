#include "KnobComponent.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"

KnobComponent::KnobComponent (juce::AudioProcessorValueTreeState& apvts,
                              const juce::String& parameterID,
                              const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 15);

    // Corsa da 7:30 a 4:30, la stessa dei potenziometri reali a 300 gradi.
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                juce::MathConstants<float>::pi * 2.75f,
                                true);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions { 11.0f }).boldened());
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);
}

void KnobComponent::paint (juce::Graphics&)
{
    // Lo sfondo e' quello del pannello: il componente disegna solo i figli.
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();

    label.setBounds (bounds.removeFromTop (14));
    slider.setBounds (bounds);
}
