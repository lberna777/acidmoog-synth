#include "AccentProcessor.h"

float AccentProcessor::accentStrength (int velocity) const noexcept
{
    if (! isAccented (velocity))
        return 0.0f;

    const auto span = (float) (127 - threshold);

    // Soglia a 127: la sola velocity massima conta come accento pieno.
    if (span <= 0.0f)
        return 1.0f;

    return juce::jlimit (0.0f, 1.0f, (float) (velocity - threshold) / span);
}

float AccentProcessor::getGainMultiplier (int velocity) const noexcept
{
    // Fino a +6 dB circa a pieno accento.
    return 1.0f + amount * accentStrength (velocity);
}

float AccentProcessor::getCutoffMultiplier (int velocity) const noexcept
{
    // Fino a due ottave di apertura del filtro a pieno accento.
    return 1.0f + 3.0f * amount * accentStrength (velocity);
}
