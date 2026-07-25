#include "WaveformDisplay.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"

WaveformDisplay::WaveformDisplay()
{
    setInterceptsMouseClicks (false, false);
}

void WaveformDisplay::setBuffer (const juce::AudioBuffer<float>& buffer)
{
    sourceCopy.makeCopyOf (buffer);
    hasContent = buffer.getNumSamples() > 0;
    rebuildPeaks();
    repaint();
}

void WaveformDisplay::clear()
{
    sourceCopy.setSize (0, 0);
    peaks.clear();
    hasContent = false;
    repaint();
}

void WaveformDisplay::resized()
{
    rebuildPeaks();
}

void WaveformDisplay::rebuildPeaks()
{
    peaks.clear();

    const auto width = getWidth();
    const auto numSamples = sourceCopy.getNumSamples();

    if (! hasContent || width <= 0 || numSamples <= 0)
        return;

    peaks.reserve ((size_t) width);

    const auto samplesPerColumn = juce::jmax (1, numSamples / width);

    for (int x = 0; x < width; ++x)
    {
        const auto start = juce::jlimit (0, numSamples - 1, x * samplesPerColumn);
        const auto end = juce::jmin (numSamples, start + samplesPerColumn);

        auto minValue = 0.0f;
        auto maxValue = 0.0f;

        for (int ch = 0; ch < sourceCopy.getNumChannels(); ++ch)
        {
            const auto range = sourceCopy.findMinMax (ch, start, end - start);
            minValue = juce::jmin (minValue, range.getStart());
            maxValue = juce::jmax (maxValue, range.getEnd());
        }

        peaks.push_back ({ minValue, maxValue });
    }
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Vetro scuro dell'oscilloscopio.
    g.setColour (juce::Colour (0xff0a0d0a));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.2f);

    const auto centreY = bounds.getCentreY();

    // Linea di zero.
    g.setColour (AcidMoogLookAndFeel::ledAmber.withAlpha (0.2f));
    g.drawHorizontalLine ((int) centreY, bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    if (! hasContent || peaks.empty())
    {
        g.setFont (juce::Font (juce::FontOptions { 10.0f }));
        g.setColour (AcidMoogLookAndFeel::engraving.withAlpha (0.35f));
        g.drawText ("NO SAMPLE", bounds, juce::Justification::centred, false);
        return;
    }

    const auto halfHeight = bounds.getHeight() * 0.45f;

    g.setColour (AcidMoogLookAndFeel::ledAmber.withAlpha (0.9f));

    for (size_t x = 0; x < peaks.size(); ++x)
    {
        const auto& range = peaks[x];
        const auto top = centreY - range.getEnd() * halfHeight;
        const auto bottom = centreY - range.getStart() * halfHeight;

        g.drawVerticalLine ((int) x, juce::jmin (top, bottom), juce::jmax (top, bottom) + 0.5f);
    }
}
