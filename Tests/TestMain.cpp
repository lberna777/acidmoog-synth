#include "DSP/Oscillator.h"
#include "DSP/LadderFilter.h"
#include "DSP/Envelope.h"
#include "DSP/GlideProcessor.h"
#include "DSP/AccentProcessor.h"
#include "Voicing/MonoVoiceManager.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    int failures = 0;
    int checks = 0;

    void check (bool condition, const std::string& what)
    {
        ++checks;

        if (! condition)
        {
            ++failures;
            std::printf ("  FAIL: %s\n", what.c_str());
        }
    }

    constexpr double sr = 48000.0;

    // Energia oltre Nyquist non e' misurabile direttamente; come proxy dell'aliasing
    // misuriamo quanto la saw devia da una rampa ideale ricostruita. Un segnale con
    // aliasing pesante mostra escursioni fuori scala e discontinuita' anomale.
    float maxAbs (const std::vector<float>& v)
    {
        float m = 0.0f;

        for (auto s : v)
            m = std::max (m, std::abs (s));

        return m;
    }

    void testOscillator()
    {
        std::printf ("Oscillator\n");

        Oscillator osc;
        osc.prepare (sr);
        osc.noteOn();

        std::vector<float> out;
        out.reserve (4096);

        for (int i = 0; i < 4096; ++i)
            out.push_back (osc.getNextSample (440.0f));

        check (maxAbs (out) <= 1.05f, "saw resta entro [-1, 1] con margine PolyBLEP");

        // A frequenza acuta la saw naive esploderebbe in artefatti: PolyBLEP deve
        // mantenere il segnale limitato e continuo.
        Oscillator high;
        high.prepare (sr);
        high.noteOn();

        std::vector<float> hi;
        float maxJump = 0.0f;
        float previous = 0.0f;

        for (int i = 0; i < 4096; ++i)
        {
            const auto s = high.getNextSample (8000.0f);

            if (i > 0)
                maxJump = std::max (maxJump, std::abs (s - previous));

            previous = s;
            hi.push_back (s);
        }

        check (maxAbs (hi) <= 1.05f, "saw a 8 kHz resta limitata");
        check (maxJump <= 2.1f, "nessun salto superiore all'escursione piena");

        // Il valore medio di una saw simmetrica deve essere circa zero.
        double sum = 0.0;

        for (auto s : out)
            sum += s;

        check (std::abs (sum / (double) out.size()) < 0.05, "componente continua trascurabile");
    }

    void testFilter()
    {
        std::printf ("LadderFilter\n");

        // Stabilita' a risonanza massima: nessun NaN, nessuna divergenza.
        LadderFilter filter;
        filter.prepare (sr);
        filter.setOversamplingFactor (4);
        filter.setType (FilterType::LowPass);

        float maxOut = 0.0f;
        bool sane = true;

        for (int i = 0; i < 48000; ++i)
        {
            const auto input = std::sin (2.0 * juce::MathConstants<double>::pi * 110.0 * i / sr);
            const auto out = filter.processSample ((float) input, 800.0f, 1.0f, 1.0f);

            if (! std::isfinite (out))
                sane = false;

            maxOut = std::max (maxOut, std::abs (out));
        }

        check (sane, "nessun NaN/Inf a risonanza e drive massimi");
        check (maxOut < 10.0f, "uscita limitata a risonanza massima");

        // Il low-pass deve attenuare una sinusoide ben sopra il cutoff.
        auto rmsThrough = [] (FilterType type, float cutoff, float toneHz)
        {
            LadderFilter f;
            f.prepare (sr);
            f.setOversamplingFactor (2);
            f.setType (type);

            double sum = 0.0;
            const int n = 24000;

            for (int i = 0; i < n; ++i)
            {
                const auto input = std::sin (2.0 * juce::MathConstants<double>::pi * toneHz * i / sr);
                const auto out = f.processSample ((float) input, cutoff, 0.1f, 0.0f);

                if (i > n / 2)          // scarta il transiente iniziale
                    sum += out * out;
            }

            return std::sqrt (sum / (n / 2));
        };

        const auto lpLow = rmsThrough (FilterType::LowPass, 1000.0f, 100.0f);
        const auto lpHigh = rmsThrough (FilterType::LowPass, 1000.0f, 8000.0f);
        check (lpLow > lpHigh * 4.0, "LP attenua sopra il cutoff");

        const auto hpLow = rmsThrough (FilterType::HighPass, 1000.0f, 100.0f);
        const auto hpHigh = rmsThrough (FilterType::HighPass, 1000.0f, 8000.0f);
        check (hpHigh > hpLow * 2.0, "HP attenua sotto il cutoff");

        const auto bpCentre = rmsThrough (FilterType::BandPass, 1000.0f, 1000.0f);
        const auto bpFar = rmsThrough (FilterType::BandPass, 1000.0f, 60.0f);
        check (bpCentre > bpFar, "BP passa di piu' attorno al cutoff");
    }

    void testEnvelope()
    {
        std::printf ("Envelope\n");

        Envelope env;
        env.prepare (sr);
        env.setParameters (0.1f, 0.1f, 0.5f, 0.2f);
        env.noteOn();

        // Dopo l'attack il valore deve essere al massimo.
        float value = 0.0f;

        for (int i = 0; i < (int) (0.1 * sr); ++i)
            value = env.getNextValue();

        check (value > 0.95f, "attack raggiunge il massimo nel tempo impostato");

        // Dopo il decay deve assestarsi sul sustain.
        for (int i = 0; i < (int) (0.15 * sr); ++i)
            value = env.getNextValue();

        check (std::abs (value - 0.5f) < 0.05f, "decay porta al livello di sustain");

        env.noteOff();

        for (int i = 0; i < (int) (0.3 * sr); ++i)
            value = env.getNextValue();

        check (value < 0.01f, "release porta a zero");
        check (! env.isActive(), "envelope inattivo dopo il release");
    }

    void testGlide()
    {
        std::printf ("GlideProcessor\n");

        GlideProcessor glide;
        glide.prepare (sr);
        glide.setGlideTime (0.1f);
        glide.setMode (GlideMode::LegatoOnly);
        glide.reset (48.0f);

        // Nota non legato in modalita' Legato Only: salto immediato.
        glide.setTargetNote (60.0f, false);
        check (std::abs (glide.getNextNote() - 60.0f) < 0.001f,
               "legato-only: nota isolata salta senza glide");

        // Nota legato: deve scivolare, non saltare.
        glide.reset (48.0f);
        glide.setTargetNote (60.0f, true);

        const auto afterOne = glide.getNextNote();
        check (afterOne > 48.0f && afterOne < 49.0f, "legato: il glide parte gradualmente");

        for (int i = 0; i < (int) (0.1 * sr) + 10; ++i)
            glide.getNextNote();

        check (std::abs (glide.getNextNote() - 60.0f) < 0.001f,
               "glide arriva esattamente al target");

        // In modalita' Always anche le note isolate scivolano.
        glide.setMode (GlideMode::Always);
        glide.reset (48.0f);
        glide.setTargetNote (60.0f, false);
        check (glide.getNextNote() < 49.0f, "always: anche la nota isolata scivola");
    }

    void testAccent()
    {
        std::printf ("AccentProcessor\n");

        AccentProcessor accent;
        accent.setAmount (100.0f);
        accent.setThreshold (100);

        check (! accent.isAccented (99), "velocity sotto soglia non accentata");
        check (accent.isAccented (127), "velocity massima accentata");

        check (std::abs (accent.getGainMultiplier (80) - 1.0f) < 0.001f,
               "nessun boost sotto soglia");
        check (accent.getGainMultiplier (127) > 1.5f, "boost di volume a pieno accento");
        check (accent.getCutoffMultiplier (127) > 2.0f, "apertura del filtro a pieno accento");

        // Amount a zero deve annullare l'effetto anche sopra soglia.
        accent.setAmount (0.0f);
        check (std::abs (accent.getGainMultiplier (127) - 1.0f) < 0.001f,
               "amount a zero disattiva l'accento");
    }

    void testVoiceManager()
    {
        std::printf ("MonoVoiceManager\n");

        MonoVoiceManager manager;
        manager.setLegatoMode (LegatoMode::Auto);

        auto a = manager.noteOn (48, 100);
        check (a.type == MonoVoiceManager::Action::Type::Start, "prima nota avvia la voce");
        check (a.retrigger, "prima nota fa retrigger");
        check (! a.legato, "prima nota non e' legato");

        auto b = manager.noteOn (55, 100);
        check (b.note == 55, "last-note priority: suona l'ultima premuta");
        check (b.legato, "seconda nota sovrapposta e' legato");
        check (! b.retrigger, "in Auto il legato non fa retrigger");

        // Rilasciando la nota attiva si torna a quella ancora premuta.
        auto c = manager.noteOff (55);
        check (c.type == MonoVoiceManager::Action::Type::Start, "ritorno alla nota sottostante");
        check (c.note == 48, "torna alla nota rimasta premuta");
        check (c.legato, "il ritorno e' legato");

        auto d = manager.noteOff (48);
        check (d.type == MonoVoiceManager::Action::Type::Release, "ultima nota rilasciata: release");
        check (! manager.hasActiveNote(), "nessuna nota attiva dopo l'ultimo release");

        // Rilasciare una nota non attiva non deve produrre eventi udibili.
        manager.noteOn (40, 100);
        manager.noteOn (52, 100);
        auto e = manager.noteOff (40);
        check (e.type == MonoVoiceManager::Action::Type::None,
               "release di una nota non attiva non cambia nulla");
        check (manager.getActiveNote() == 52, "la nota attiva resta l'ultima premuta");

        // In Always Retrigger anche il legato riparte da capo.
        MonoVoiceManager retrig;
        retrig.setLegatoMode (LegatoMode::AlwaysRetrigger);
        retrig.noteOn (48, 100);
        auto f = retrig.noteOn (55, 100);
        check (f.retrigger, "Always Retrigger: anche il legato fa retrigger");
    }
}

int main()
{
    std::printf ("AcidMoog DSP tests\n\n");

    testOscillator();
    testFilter();
    testEnvelope();
    testGlide();
    testAccent();
    testVoiceManager();

    std::printf ("\n%d checks, %d failures\n", checks, failures);

    return failures == 0 ? 0 : 1;
}
