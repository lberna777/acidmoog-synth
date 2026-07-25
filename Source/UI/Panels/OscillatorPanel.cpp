#include "OscillatorPanel.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"
#include "../../PluginProcessor.h"

OscillatorPanel::OscillatorPanel (AcidMoogAudioProcessor& processor)
    : audioProcessor (processor),
      sourceSwitch (processor.getAPVTS(), ParamID::oscType, "Source", { "SYNTH", "SMPL" }),
      rootNoteKnob (processor.getAPVTS(), ParamID::sampleRootNote, "Root")
{
    addAndMakeVisible (sourceSwitch);
    addAndMakeVisible (rootNoteKnob);
    addAndMakeVisible (waveform);
    addAndMakeVisible (dropZone);

    dropZone.onFileChosen = [this] (const juce::File& f) { loadFile (f); };

    // Se il plugin viene riaperto con un sample gia' caricato, la UI lo riflette.
    if (audioProcessor.getVoice().getSampler().hasSample())
    {
        waveform.setBuffer (audioProcessor.getVoice().getSampler().getBuffer());
        dropZone.setStatusText (audioProcessor.getLoadedSampleName());
    }

    updateModeVisibility();
    startTimerHz (10);
}

void OscillatorPanel::loadFile (const juce::File& file)
{
    if (audioProcessor.loadSampleFile (file))
    {
        waveform.setBuffer (audioProcessor.getVoice().getSampler().getBuffer());
        dropZone.setStatusText (file.getFileName());
    }
    else
    {
        dropZone.setStatusText ("LOAD FAILED");
    }
}

void OscillatorPanel::timerCallback()
{
    updateModeVisibility();
}

void OscillatorPanel::updateModeVisibility()
{
    const auto isSampler = audioProcessor.getAPVTS()
                              .getRawParameterValue (ParamID::oscType)->load() > 0.5f;

    if (isSampler == samplerMode)
        return;

    samplerMode = isSampler;

    // In modalita' Synth c'e' solo la saw, per design: i controlli del sampler
    // spariscono invece di restare visibili ma inerti.
    rootNoteKnob.setVisible (isSampler);
    waveform.setVisible (isSampler);
    dropZone.setVisible (isSampler);

    repaint();
}

void OscillatorPanel::paint (juce::Graphics& g)
{
    AcidMoogLookAndFeel::paintSectionFrame (g, getLocalBounds().toFloat(), "Source");

    if (! samplerMode)
    {
        // Anteprima statica della saw: comunica che in questa modalita' la forma
        // d'onda e' fissa.
        auto area = getLocalBounds().toFloat().reduced (16.0f);
        area.removeFromTop (60.0f);

        juce::Path saw;
        const auto cycles = 3.0f;
        const auto midY = area.getCentreY();
        const auto amp = juce::jmin (26.0f, area.getHeight() * 0.35f);

        for (int i = 0; i <= (int) cycles; ++i)
        {
            const auto x0 = area.getX() + area.getWidth() * (float) i / cycles;
            const auto x1 = area.getX() + area.getWidth() * (float) (i + 1) / cycles;

            saw.startNewSubPath (x0, midY + amp);
            saw.lineTo (juce::jmin (x1, area.getRight()), midY - amp);
            saw.lineTo (juce::jmin (x1, area.getRight()), midY + amp);
        }

        g.setColour (AcidMoogLookAndFeel::ledAmber.withAlpha (0.55f));
        g.strokePath (saw, juce::PathStrokeType (1.6f));

        g.setFont (juce::Font (juce::FontOptions { 10.0f }));
        g.setColour (AcidMoogLookAndFeel::engraving.withAlpha (0.6f));
        g.drawText ("SAWTOOTH", area.removeFromBottom (14.0f),
                    juce::Justification::centred, false);
    }
}

void OscillatorPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    bounds.removeFromTop (14);   // spazio per il titolo della sezione

    sourceSwitch.setBounds (bounds.removeFromTop (46));
    bounds.removeFromTop (8);

    // I controlli del sampler occupano sempre lo stesso spazio: cambiare modalita'
    // ne commuta solo la visibilita', senza rimescolare il layout.
    dropZone.setBounds (bounds.removeFromTop (36));
    bounds.removeFromTop (6);
    waveform.setBounds (bounds.removeFromTop (52));
    bounds.removeFromTop (6);
    rootNoteKnob.setBounds (bounds.removeFromTop (66).withSizeKeepingCentre (66, 66));
}
