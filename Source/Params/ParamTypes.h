#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/** Tipi dei parametri condivisi fra DSP e UI.

    Sta separato da ParameterLayout.h di proposito: i moduli DSP hanno bisogno solo
    di questi enum, non dell'intero juce_audio_processors (e quindi del modulo
    grafico), cosi' restano compilabili e testabili da soli.
*/

enum class OscType    { Synth = 0, Sampler = 1 };
enum class FilterType { LowPass = 0, HighPass = 1, BandPass = 2 };
enum class GlideMode  { Always = 0, LegatoOnly = 1 };
enum class LegatoMode { Auto = 0, AlwaysRetrigger = 1 };
