#pragma once

#include "PluginProcessor.h"
#include "UI/LookAndFeel/AcidMoogLookAndFeel.h"
#include "UI/Panels/OscillatorPanel.h"
#include "UI/Panels/FilterPanel.h"
#include "UI/Panels/EnvelopePanel.h"
#include "UI/Panels/PerformancePanel.h"

class AcidMoogAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AcidMoogAudioProcessorEditor (AcidMoogAudioProcessor&);
    ~AcidMoogAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AcidMoogAudioProcessor& audioProcessor;
    AcidMoogLookAndFeel lookAndFeel;

    OscillatorPanel oscillatorPanel;
    FilterPanel filterPanel;
    EnvelopePanel envelopePanel;
    PerformancePanel performancePanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidMoogAudioProcessorEditor)
};
