#include "FilterPanel.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"
#include "../../PluginProcessor.h"

FilterPanel::FilterPanel (AcidMoogAudioProcessor& processor)
    : audioProcessor (processor),
      typeSwitch (processor.getAPVTS(), ParamID::filterType, "Mode", { "LP", "HP", "BP" }),
      cutoffKnob (processor.getAPVTS(), ParamID::cutoff, "Cutoff"),
      resonanceKnob (processor.getAPVTS(), ParamID::resonance, "Resonance"),
      driveKnob (processor.getAPVTS(), ParamID::drive, "Drive"),
      envAmountKnob (processor.getAPVTS(), ParamID::envToFilterAmount, "Env > Filter"),
      selfOscLed (AcidMoogLookAndFeel::ledRed, "OSC")
{
    addAndMakeVisible (typeSwitch);
    addAndMakeVisible (cutoffKnob);
    addAndMakeVisible (resonanceKnob);
    addAndMakeVisible (driveKnob);
    addAndMakeVisible (envAmountKnob);
    addAndMakeVisible (selfOscLed);

    startTimerHz (20);
}

void FilterPanel::timerCallback()
{
    selfOscLed.setState (audioProcessor.getVoice().isSelfOscillating());
}

void FilterPanel::paint (juce::Graphics& g)
{
    AcidMoogLookAndFeel::paintSectionFrame (g, getLocalBounds().toFloat(), "Filter");
}

void FilterPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    bounds.removeFromTop (14);

    auto topRow = bounds.removeFromTop (46);
    typeSwitch.setBounds (topRow.removeFromLeft (topRow.getWidth() - 44));
    selfOscLed.setBounds (topRow.reduced (6, 2));

    bounds.removeFromTop (10);

    // Cutoff e Resonance grandi e affiancati: sono i controlli che si suonano.
    auto bigRow = bounds.removeFromTop (112);
    const auto bigWidth = bigRow.getWidth() / 2;
    cutoffKnob.setBounds (bigRow.removeFromLeft (bigWidth).reduced (6));
    resonanceKnob.setBounds (bigRow.reduced (6));

    bounds.removeFromTop (6);

    auto smallRow = bounds.removeFromTop (86);
    const auto smallWidth = smallRow.getWidth() / 2;
    driveKnob.setBounds (smallRow.removeFromLeft (smallWidth).reduced (10, 4));
    envAmountKnob.setBounds (smallRow.reduced (10, 4));
}
