#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/** Look and feel del pannello: metallo scuro, manopole tornite, serigrafia chiara.

    Le manopole sono disegnate proceduralmente invece che con sprite pre-renderizzate:
    stessa resa "hardware" (gradienti, ombra portata, smusso, indicatore inciso) ma
    scalabile a qualsiasi dimensione e senza asset binari da mantenere.
*/
class AcidMoogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AcidMoogLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

    /** Sfondo pannello con texture metallica spazzolata e vignettatura. */
    static void paintPanelBackground (juce::Graphics&, juce::Rectangle<int> bounds);

    /** Cornice incisa usata per delimitare le sezioni del pannello. */
    static void paintSectionFrame (juce::Graphics&, juce::Rectangle<float> bounds,
                                   const juce::String& title);

    static void paintScrew (juce::Graphics&, juce::Point<float> centre, float radius);

    // Palette condivisa da tutti i componenti.
    static const juce::Colour panelDark;
    static const juce::Colour panelLight;
    static const juce::Colour engraving;
    static const juce::Colour ledRed;
    static const juce::Colour ledAmber;
    static const juce::Colour accentOrange;
};
