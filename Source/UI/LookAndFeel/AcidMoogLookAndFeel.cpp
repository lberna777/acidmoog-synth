#include "AcidMoogLookAndFeel.h"

const juce::Colour AcidMoogLookAndFeel::panelDark     { 0xff17151a };
const juce::Colour AcidMoogLookAndFeel::panelLight    { 0xff2e2a30 };
const juce::Colour AcidMoogLookAndFeel::engraving     { 0xffd8d2c4 };
const juce::Colour AcidMoogLookAndFeel::ledRed        { 0xffff3b24 };
const juce::Colour AcidMoogLookAndFeel::ledAmber      { 0xffffa41f };
const juce::Colour AcidMoogLookAndFeel::accentOrange  { 0xffe07b2c };

AcidMoogLookAndFeel::AcidMoogLookAndFeel()
{
    setColour (juce::Label::textColourId, engraving);
    setColour (juce::Slider::textBoxTextColourId, engraving);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff0d0c0f));
    setColour (juce::ComboBox::backgroundColourId, panelDark);
    setColour (juce::ComboBox::textColourId, engraving);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff4a464e));
    setColour (juce::PopupMenu::backgroundColourId, panelDark);
    setColour (juce::PopupMenu::textColourId, engraving);
}

void AcidMoogLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Ghiera incisa con tacche: da' la scala di riferimento come sull'hardware.
    {
        const auto tickRadius = radius * 0.98f;
        g.setColour (engraving.withAlpha (0.45f));

        for (int i = 0; i <= 10; ++i)
        {
            const auto t = rotaryStartAngle + (float) i / 10.0f * (rotaryEndAngle - rotaryStartAngle);
            const auto inner = centre.getPointOnCircumference (tickRadius * 0.88f, t);
            const auto outer = centre.getPointOnCircumference (tickRadius, t);
            g.drawLine ({ inner, outer }, i % 5 == 0 ? 1.6f : 0.9f);
        }
    }

    const auto knobRadius = radius * 0.78f;

    // Ombra portata: la manopola deve sembrare sollevata dal pannello.
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f)
                       .withCentre (centre.translated (1.5f, 2.5f)));

    // Corpo: gradiente diagonale che simula il metallo tornito illuminato dall'alto.
    const auto knobArea = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f)
                              .withCentre (centre);

    juce::ColourGradient body (juce::Colour (0xff5a565f), knobArea.getTopLeft(),
                               juce::Colour (0xff1b191d), knobArea.getBottomRight(), false);
    body.addColour (0.45, juce::Colour (0xff3a373f));
    g.setGradientFill (body);
    g.fillEllipse (knobArea);

    // Smusso chiaro in alto e scuro in basso: il bordo "prende luce".
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawEllipse (knobArea.reduced (0.8f), 1.4f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (knobArea.reduced (2.4f), 1.0f);

    // Cappuccio interno piu' scuro, come le manopole Moog a due livelli.
    const auto capArea = knobArea.reduced (knobRadius * 0.34f);
    juce::ColourGradient cap (juce::Colour (0xff2b282e), capArea.getTopLeft(),
                              juce::Colour (0xff101013), capArea.getBottomRight(), false);
    g.setGradientFill (cap);
    g.fillEllipse (capArea);

    // Indicatore inciso, con la scanalatura scura sotto la linea chiara.
    const auto pointerStart = centre.getPointOnCircumference (knobRadius * 0.32f, angle);
    const auto pointerEnd = centre.getPointOnCircumference (knobRadius * 0.92f, angle);

    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.drawLine ({ pointerStart.translated (0.0f, 1.0f), pointerEnd.translated (0.0f, 1.0f) }, 3.2f);
    g.setColour (engraving);
    g.drawLine ({ pointerStart, pointerEnd }, 2.2f);
}

void AcidMoogLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    const auto bounds = label.getLocalBounds().toFloat();

    g.setFont (label.getFont());

    // Testo serigrafato: ombra scura sotto per staccarlo dal metallo.
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawText (label.getText(), bounds.translated (0.0f, 1.0f).toNearestInt(),
                label.getJustificationType(), false);

    g.setColour (label.findColour (juce::Label::textColourId));
    g.drawText (label.getText(), bounds.toNearestInt(), label.getJustificationType(), false);
}

void AcidMoogLookAndFeel::paintPanelBackground (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const auto area = bounds.toFloat();

    juce::ColourGradient base (panelLight, area.getCentreX(), area.getY(),
                               panelDark, area.getCentreX(), area.getBottom(), false);
    g.setGradientFill (base);
    g.fillRect (area);

    // Spazzolatura orizzontale: righe sottili a opacita' pseudo-casuale ma stabile,
    // cosi' la texture non "sfarfalla" fra un repaint e l'altro.
    juce::Random rng (0x4c1d5e);
    g.saveState();

    for (int yPos = 0; yPos < bounds.getHeight(); yPos += 2)
    {
        const auto alpha = 0.02f + rng.nextFloat() * 0.035f;
        g.setColour (juce::Colours::white.withAlpha (alpha));
        g.drawHorizontalLine (yPos, area.getX(), area.getRight());
    }

    g.restoreState();

    // Vignettatura: bordi piu' scuri, come una foto di un pannello reale.
    juce::ColourGradient vignette (juce::Colours::transparentBlack, area.getCentre(),
                                   juce::Colours::black.withAlpha (0.55f), area.getTopLeft(), true);
    g.setGradientFill (vignette);
    g.fillRect (area);

    const auto screwInset = 16.0f;
    paintScrew (g, { area.getX() + screwInset, area.getY() + screwInset }, 6.0f);
    paintScrew (g, { area.getRight() - screwInset, area.getY() + screwInset }, 6.0f);
    paintScrew (g, { area.getX() + screwInset, area.getBottom() - screwInset }, 6.0f);
    paintScrew (g, { area.getRight() - screwInset, area.getBottom() - screwInset }, 6.0f);
}

void AcidMoogLookAndFeel::paintSectionFrame (juce::Graphics& g, juce::Rectangle<float> bounds,
                                             const juce::String& title)
{
    const auto frame = bounds.reduced (2.0f);

    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillRoundedRectangle (frame, 6.0f);

    // Doppio tratto (scuro sotto, chiaro sopra) per l'effetto inciso nel metallo.
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (frame.translated (0.0f, 1.0f), 6.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (frame, 6.0f, 1.2f);

    if (title.isNotEmpty())
    {
        const auto titleArea = frame.withHeight (18.0f).reduced (10.0f, 0.0f);

        g.setFont (juce::Font (juce::FontOptions { 11.0f }).boldened());
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.drawText (title.toUpperCase(), titleArea.translated (0.0f, 1.0f),
                    juce::Justification::centredLeft, false);
        g.setColour (engraving.withAlpha (0.85f));
        g.drawText (title.toUpperCase(), titleArea, juce::Justification::centredLeft, false);
    }
}

void AcidMoogLookAndFeel::paintScrew (juce::Graphics& g, juce::Point<float> centre, float radius)
{
    const auto area = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (area.translated (0.0f, 1.0f));

    juce::ColourGradient metal (juce::Colour (0xff6f6b73), area.getTopLeft(),
                                juce::Colour (0xff232126), area.getBottomRight(), false);
    g.setGradientFill (metal);
    g.fillEllipse (area);

    // Taglio della vite, leggermente ruotato: le viti reali non sono mai allineate.
    juce::Path slot;
    slot.addRectangle (-radius * 0.7f, -0.9f, radius * 1.4f, 1.8f);
    slot.applyTransform (juce::AffineTransform::rotation (0.6f).translated (centre));

    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.fillPath (slot);
}
