#include "ToggleSwitchComponent.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"

ToggleSwitchComponent::ToggleSwitchComponent (juce::AudioProcessorValueTreeState& apvts,
                                              const juce::String& parameterID,
                                              const juce::String& labelText,
                                              juce::StringArray positionNames)
    : state (apvts), paramID (parameterID), title (labelText),
      positions (std::move (positionNames))
{
    paramValue = state.getParameterAsValue (paramID);
    paramValue.addListener (this);
}

int ToggleSwitchComponent::getCurrentIndex() const
{
    if (auto* param = state.getParameter (paramID))
        return param->convertFrom0to1 (param->getValue());

    return 0;
}

void ToggleSwitchComponent::mouseDown (const juce::MouseEvent&)
{
    auto* param = state.getParameter (paramID);

    if (param == nullptr || positions.isEmpty())
        return;

    const auto next = (getCurrentIndex() + 1) % positions.size();

    param->beginChangeGesture();
    param->setValueNotifyingHost (param->convertTo0to1 ((float) next));
    param->endChangeGesture();

    repaint();
}

void ToggleSwitchComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    auto titleArea = bounds.removeFromTop (14.0f);
    g.setFont (juce::Font (juce::FontOptions { 11.0f }).boldened());
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawText (title, titleArea.translated (0.0f, 1.0f), juce::Justification::centred, false);
    g.setColour (AcidMoogLookAndFeel::engraving);
    g.drawText (title, titleArea, juce::Justification::centred, false);

    const auto body = bounds.reduced (4.0f);

    // Incasso scuro in cui alloggia la levetta.
    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.fillRoundedRectangle (body, 4.0f);
    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawRoundedRectangle (body, 4.0f, 1.0f);

    const auto index = juce::jlimit (0, juce::jmax (0, positions.size() - 1), getCurrentIndex());
    const auto slotWidth = body.getWidth() / (float) juce::jmax (1, positions.size());
    const auto knobArea = body.withWidth (slotWidth).translated (slotWidth * (float) index, 0.0f)
                              .reduced (2.0f);

    juce::ColourGradient metal (juce::Colour (0xff6c6871), knobArea.getTopLeft(),
                                juce::Colour (0xff232025), knobArea.getBottomRight(), false);
    g.setGradientFill (metal);
    g.fillRoundedRectangle (knobArea, 3.0f);

    g.setColour (juce::Colours::white.withAlpha (0.2f));
    g.drawRoundedRectangle (knobArea, 3.0f, 1.0f);

    // Nomi delle posizioni: quella attiva in ambra, le altre spente.
    g.setFont (juce::Font (juce::FontOptions { 10.0f }));

    for (int i = 0; i < positions.size(); ++i)
    {
        const auto slot = body.withWidth (slotWidth).translated (slotWidth * (float) i, 0.0f);

        g.setColour (i == index ? AcidMoogLookAndFeel::ledAmber
                                : AcidMoogLookAndFeel::engraving.withAlpha (0.4f));
        g.drawText (positions[i], slot, juce::Justification::centred, false);
    }
}
