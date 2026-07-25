#pragma once

#include "../Params/ParamTypes.h"

/** Portamento monofonico fra note.

    Il glide lavora in semitoni (non in Hz): scivolare linearmente in frequenza
    darebbe una curva percettivamente sbagliata, con l'ottava bassa molto piu' lenta.
*/
class GlideProcessor
{
public:
    void prepare (double sampleRate);
    void reset (float noteNumber);

    void setGlideTime (float seconds) noexcept { glideSeconds = juce::jmax (0.0f, seconds); }
    void setMode (GlideMode m) noexcept        { mode = m; }

    /** Imposta la nota di destinazione. isLegato distingue una nota suonata sopra
        una precedente ancora premuta da un attacco isolato: in Legato Only il glide
        agisce solo nel primo caso. */
    void setTargetNote (float noteNumber, bool isLegato) noexcept;

    /** Avanza di un campione e restituisce il numero di nota MIDI corrente (float). */
    float getNextNote() noexcept;

private:
    double currentSampleRate { 44100.0 };
    float glideSeconds { 0.08f };
    GlideMode mode { GlideMode::LegatoOnly };

    float currentNote { 60.0f };
    float targetNote { 60.0f };
    float incrementPerSample { 0.0f };
};
