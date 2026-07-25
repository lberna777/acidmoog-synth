#include "LedComponent.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"

LedComponent::LedComponent (juce::Colour colour, const juce::String& labelText)
    : ledColour (colour), title (labelText)
{
    startTimerHz (30);
}

void LedComponent::timerCallback()
{
    // Salita rapida, discesa piu' lenta: e' il comportamento che rende leggibile
    // un lampeggio breve come quello dell'accento.
    const auto rate = target > brightness ? 0.45f : 0.12f;
    const auto next = brightness + (target - brightness) * rate;

    if (std::abs (next - brightness) > 0.001f)
    {
        brightness = next;
        repaint();
    }
}

void LedComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    const auto ledSize = juce::jmin (bounds.getWidth(), bounds.getHeight() - 12.0f);
    const auto ledArea = juce::Rectangle<float> (ledSize, ledSize)
                             .withCentre ({ bounds.getCentreX(), bounds.getY() + ledSize * 0.5f + 2.0f });

    // Alone: cresce con la luminosita', e' cio' che vende l'effetto "acceso".
    if (brightness > 0.01f)
    {
        g.setColour (ledColour.withAlpha (0.28f * brightness));
        g.fillEllipse (ledArea.expanded (ledSize * 0.7f * brightness));
    }

    // Alloggiamento metallico incassato.
    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.fillEllipse (ledArea.expanded (2.0f));

    const auto lit = ledColour.withMultipliedBrightness (0.25f + 0.75f * brightness)
                              .withMultipliedSaturation (0.6f + 0.4f * brightness);

    juce::ColourGradient dome (lit.brighter (0.5f), ledArea.getCentreX(), ledArea.getY(),
                               lit.darker (0.7f), ledArea.getCentreX(), ledArea.getBottom(), false);
    g.setGradientFill (dome);
    g.fillEllipse (ledArea);

    // Riflesso speculare in alto a sinistra.
    g.setColour (juce::Colours::white.withAlpha (0.35f + 0.3f * brightness));
    g.fillEllipse (ledArea.withSizeKeepingCentre (ledSize * 0.3f, ledSize * 0.24f)
                          .translated (-ledSize * 0.16f, -ledSize * 0.2f));

    if (title.isNotEmpty())
    {
        auto textArea = bounds.removeFromBottom (11.0f);
        g.setFont (juce::Font (juce::FontOptions { 9.0f }));
        g.setColour (AcidMoogLookAndFeel::engraving.withAlpha (0.75f));
        g.drawText (title, textArea, juce::Justification::centred, false);
    }
}
