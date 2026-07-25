#include "PluginEditor.h"

namespace
{
    constexpr int editorWidth = 880;
    constexpr int editorHeight = 520;
    constexpr int headerHeight = 54;
}

AcidMoogAudioProcessorEditor::AcidMoogAudioProcessorEditor (AcidMoogAudioProcessor& p)
    : juce::AudioProcessorEditor (&p),
      audioProcessor (p),
      oscillatorPanel (p),
      filterPanel (p),
      envelopePanel (p),
      performancePanel (p)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (oscillatorPanel);
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (envelopePanel);
    addAndMakeVisible (performancePanel);

    setSize (editorWidth, editorHeight);
}

AcidMoogAudioProcessorEditor::~AcidMoogAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AcidMoogAudioProcessorEditor::paint (juce::Graphics& g)
{
    AcidMoogLookAndFeel::paintPanelBackground (g, getLocalBounds());

    // Targhetta del marchio, come la serigrafia su un pannello reale.
    auto header = getLocalBounds().removeFromTop (headerHeight).reduced (28, 8);

    g.setFont (juce::Font (juce::FontOptions { 26.0f }).boldened());
    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.drawText ("ACIDMOOG", header.translated (0, 2), juce::Justification::centredLeft, false);
    g.setColour (AcidMoogLookAndFeel::engraving);
    g.drawText ("ACIDMOOG", header, juce::Justification::centredLeft, false);

    g.setFont (juce::Font (juce::FontOptions { 10.0f }));
    g.setColour (AcidMoogLookAndFeel::accentOrange);
    g.drawText ("MONOPHONIC BASS SYNTHESIZER", header,
                juce::Justification::centredRight, false);

    // Riga incisa che separa header e pannello controlli.
    const auto lineY = (float) headerHeight - 4.0f;
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawLine (16.0f, lineY, (float) getWidth() - 16.0f, lineY, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawLine (16.0f, lineY + 1.2f, (float) getWidth() - 16.0f, lineY + 1.2f, 1.0f);
}

void AcidMoogAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop (headerHeight);
    bounds.reduce (12, 8);

    // Colonna sinistra: sorgente. Colonna destra: filtro sopra, envelope e
    // performance sotto — l'ordine segue il flusso del segnale.
    auto left = bounds.removeFromLeft (232);
    oscillatorPanel.setBounds (left.reduced (4));

    auto right = bounds;
    filterPanel.setBounds (right.removeFromLeft (300).reduced (4));

    envelopePanel.setBounds (right.removeFromTop (right.getHeight() / 2).reduced (4));
    performancePanel.setBounds (right.reduced (4));
}
