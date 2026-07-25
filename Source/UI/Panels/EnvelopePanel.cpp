#include "EnvelopePanel.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"
#include "../../PluginProcessor.h"

EnvelopePanel::EnvelopePanel (AcidMoogAudioProcessor& processor)
    : attackKnob (processor.getAPVTS(), ParamID::attack, "Attack"),
      decayKnob (processor.getAPVTS(), ParamID::decay, "Decay"),
      sustainKnob (processor.getAPVTS(), ParamID::sustain, "Sustain"),
      releaseKnob (processor.getAPVTS(), ParamID::release, "Release")
{
    addAndMakeVisible (attackKnob);
    addAndMakeVisible (decayKnob);
    addAndMakeVisible (sustainKnob);
    addAndMakeVisible (releaseKnob);
}

void EnvelopePanel::paint (juce::Graphics& g)
{
    AcidMoogLookAndFeel::paintSectionFrame (g, getLocalBounds().toFloat(), "Envelope");
}

void EnvelopePanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    bounds.removeFromTop (16);

    KnobComponent* knobs[] = { &attackKnob, &decayKnob, &sustainKnob, &releaseKnob };

    auto row = bounds.removeFromTop (juce::jmin (96, bounds.getHeight()));
    const auto width = row.getWidth() / 4;

    for (auto* knob : knobs)
        knob->setBounds (row.removeFromLeft (width).reduced (4));
}
