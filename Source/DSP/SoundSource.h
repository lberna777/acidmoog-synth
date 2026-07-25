#pragma once

/** Interfaccia comune fra oscillatore sintetico e sampler.

    La Voice non sa quale delle due implementazioni sta usando: chiede un campione
    alla volta a una frequenza data, così lo switch Synth/Sampler resta un semplice
    cambio di puntatore (con crossfade gestito dalla Voice).
*/
class SoundSource
{
public:
    virtual ~SoundSource() = default;

    virtual void prepare (double sampleRate) = 0;

    /** Resetta la fase / posizione di lettura all'inizio di una nuova nota. */
    virtual void noteOn() = 0;

    /** Produce il prossimo campione alla frequenza indicata (Hz, gia' glide-corretta). */
    virtual float getNextSample (float frequencyHz) = 0;
};
