#include "PerformancePanel.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"
#include "../../PluginProcessor.h"

PerformancePanel::PerformancePanel (AcidMoogAudioProcessor& processor)
    : audioProcessor (processor),
      accentAmountKnob (processor.getAPVTS(), ParamID::accentAmount, "Accent"),
      accentThresholdKnob (processor.getAPVTS(), ParamID::accentThreshold, "Thresh"),
      glideTimeKnob (processor.getAPVTS(), ParamID::glideTime, "Glide"),
      pitchBendKnob (processor.getAPVTS(), ParamID::pitchBendRange, "Bend"),
      volumeKnob (processor.getAPVTS(), ParamID::masterVolume, "Volume"),
      glideModeSwitch (processor.getAPVTS(), ParamID::glideMode, "Glide Mode", { "ALW", "LEG" }),
      legatoModeSwitch (processor.getAPVTS(), ParamID::legatoMode, "Legato", { "AUTO", "RETR" }),
      accentLed (AcidMoogLookAndFeel::ledAmber, "ACC")
{
    addAndMakeVisible (accentAmountKnob);
    addAndMakeVisible (accentThresholdKnob);
    addAndMakeVisible (glideTimeKnob);
    addAndMakeVisible (pitchBendKnob);
    addAndMakeVisible (volumeKnob);
    addAndMakeVisible (glideModeSwitch);
    addAndMakeVisible (legatoModeSwitch);
    addAndMakeVisible (accentLed);

    startTimerHz (20);
}

void PerformancePanel::timerCallback()
{
    accentLed.setState (audioProcessor.getVoice().isAccentActive());
}

void PerformancePanel::paint (juce::Graphics& g)
{
    AcidMoogLookAndFeel::paintSectionFrame (g, getLocalBounds().toFloat(), "Performance");
}

void PerformancePanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    bounds.removeFromTop (16);

    auto knobRow = bounds.removeFromTop (92);
    const auto knobWidth = knobRow.getWidth() / 5;

    accentAmountKnob.setBounds (knobRow.removeFromLeft (knobWidth).reduced (4));
    accentThresholdKnob.setBounds (knobRow.removeFromLeft (knobWidth).reduced (4));
    glideTimeKnob.setBounds (knobRow.removeFromLeft (knobWidth).reduced (4));
    pitchBendKnob.setBounds (knobRow.removeFromLeft (knobWidth).reduced (4));
    volumeKnob.setBounds (knobRow.reduced (4));

    bounds.removeFromTop (8);

    auto switchRow = bounds.removeFromTop (46);
    const auto switchWidth = (switchRow.getWidth() - 48) / 2;

    glideModeSwitch.setBounds (switchRow.removeFromLeft (switchWidth).reduced (4, 0));
    legatoModeSwitch.setBounds (switchRow.removeFromLeft (switchWidth).reduced (4, 0));
    accentLed.setBounds (switchRow.reduced (4, 2));
}
