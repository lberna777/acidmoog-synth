#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

/** Display della forma d'onda del sample caricato, in stile oscilloscopio.

    Il disegno e' statico: viene ricalcolato solo quando cambia il sample, non a
    ogni frame, cosi' la UI non pesa sulla CPU durante la riproduzione.
*/
class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay();

    /** Ricalcola i picchi dal buffer (chiamare dal message thread). */
    void setBuffer (const juce::AudioBuffer<float>& buffer);
    void clear();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void rebuildPeaks();

    juce::AudioBuffer<float> sourceCopy;
    std::vector<juce::Range<float>> peaks;   // min/max per colonna di pixel
    bool hasContent { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
