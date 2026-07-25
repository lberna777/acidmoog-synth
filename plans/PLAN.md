# Plugin Bass Synth — "AcidMoog" (nome provvisorio)

## Context

Lorenzo (music producer, FL Studio/trap, in fase di miglioramento mixing) vuole un plugin VST3/AU per Ableton e Logic: un sintetizzatore di bassi ibrido tra TB-303 e Moog, con UI fotorealistica in stile hardware vintage. Il piano è scritto in forma tecnica e densa, non didattica: ogni parametro e comportamento è dichiarato esplicitamente per evitare decisioni di design implicite.

Decisioni chiave raccolte in brainstorming:
- Framework: **JUCE (C++)**, target VST3 + AU da singolo codebase.
- Licensing: JUCE richiede licenza commerciale se il plugin non è distribuito open-source e si superano le soglie gratuite (JUCE Personal/Indie) — da verificare prima del packaging finale, non blocca lo sviluppo.
- Motore sonoro: **un solo oscillatore Saw** (no multi-osc, no altre waveform), band-limited (PolyBLEP).
- Modalità **Sampler**: alternativa al synth engine, single-sample pitch-tracked su tutta la tastiera (root note configurabile), stessa catena filtro/envelope del synth.
- Filtro: **selezionabile LP/HP/BP**, ladder Moog/303 con risonanza autooscillante e drive/saturazione pre-filtro, **oversampled internamente (2x-4x)**.
- Inviluppo: ADSR su ampiezza, instradabile al filtro con quantità dedicata (`envToFilterAmount`).
- Feature acid: **Accent** (via velocity, soglia configurabile, enfatizza volume+cutoff) e **Glide/portamento** monofonico, con modalità esplicita (always vs legato-only).
- Voicing: **monofonico**, **last-note priority**; retrigger dell'envelope su note non legate, mantenimento su note legate.
- UI: **vintage analogico scuro**, fotorealistico. Switch/selettori gestiti click-free anche a nota in corso.

## Architettura del progetto (moduli e file)

```
acidmoog-synth/
├── CMakeLists.txt                     # build JUCE via add_subdirectory
├── Source/
│   ├── PluginProcessor.h/.cpp         # AudioProcessor: entry point host, buffer audio e MIDI
│   ├── PluginEditor.h/.cpp            # AudioProcessorEditor: root della UI
│   ├── Params/
│   │   └── ParameterLayout.h/.cpp     # APVTS: definizione unica dei parametri (vedi tabella)
│   ├── DSP/
│   │   ├── Voice.h/.cpp               # voce mono: source, envelope, filtro, glide; SmoothedValue sui continui
│   │   ├── Oscillator.h/.cpp          # Saw band-limited (PolyBLEP)
│   │   ├── SamplerSource.h/.cpp       # caricamento sample, pitch-tracking, interpolazione di qualità
│   │   ├── SoundSource.h              # interfaccia comune Oscillator/SamplerSource
│   │   ├── LadderFilter.h/.cpp        # ladder 24dB/oct, LP/HP/BP, risonanza autooscillante, drive, oversampling
│   │   ├── Envelope.h/.cpp            # ADSR
│   │   ├── GlideProcessor.h/.cpp      # portamento mono (glideTime + glideMode)
│   │   └── AccentProcessor.h/.cpp     # velocity -> accent (threshold/amount)
│   ├── Voicing/
│   │   └── MonoVoiceManager.h/.cpp    # last-note priority, legato/retrigger, pitch bend
│   └── UI/
│       ├── LookAndFeel/AcidMoogLookAndFeel.h/.cpp
│       ├── Components/{KnobComponent,ToggleSwitchComponent,LedComponent,WaveformDisplay,FileDropZone}.h/.cpp
│       └── Panels/{OscillatorPanel,FilterPanel,EnvelopePanel,PerformancePanel}.h/.cpp
├── Assets/Images/
└── Tests/                             # DSP unit test (oscillatore, filtro, envelope, glide, accent, voice manager)
```

### Flusso dati
1. MIDI dall'host → `PluginProcessor::processBlock` → `MonoVoiceManager` risolve la nota attiva (last-note priority, stato legato/retrigger).
2. `MonoVoiceManager` aggiorna il target pitch su `GlideProcessor` (rispettando `glideMode`) e applica il pitch bend entro `pitchBendRange`.
3. `Voice` legge da `SoundSource` (Oscillator o SamplerSource secondo `oscType`, con crossfade click-free al cambio) alla frequenza glide-corretta.
4. Il segnale passa in `LadderFilter`: drive pre-filtro, cutoff modulato dall'envelope via `envToFilterAmount`, catena oversampled 2x-4x.
5. `Envelope` modula l'ampiezza; `AccentProcessor` applica boost su volume/cutoff se velocity ≥ `accentThreshold`.
6. Tutti i parametri continui passano da `juce::SmoothedValue` (no zipper noise).
7. Output nel buffer, gestito da `PluginProcessor`.

Tutti i parametri passano da un unico `AudioProcessorValueTreeState` (APVTS) definito in `ParameterLayout`: UI e DSP restano disaccoppiati e l'automazione host funziona out-of-the-box.

## Parametri (APVTS) — elenco completo

| ID                  | Tipo    | Range / opzioni           | Default | Note |
|---------------------|---------|---------------------------|---------|------|
| oscType             | choice  | Synth / Sampler           | Synth   | crossfade click-free al cambio |
| filterType          | choice  | LP / HP / BP              | LP      | selettore fisico 3 posizioni |
| cutoff              | float   | 20 Hz – 20 kHz (log skew) | 1 kHz   | smoothed |
| resonance           | float   | 0 – 100%                  | 20%     | autooscillazione oltre ~90%, smoothed |
| drive               | float   | 0 – 100%                  | 0%      | smoothed, pre-filtro |
| attack              | float   | 0 – 5 s (skew)            | 1 ms    | |
| decay               | float   | 0 – 5 s (skew)            | 150 ms  | |
| sustain             | float   | 0 – 100%                  | 70%     | |
| release             | float   | 0 – 5 s (skew)            | 100 ms  | |
| envToFilterAmount   | float   | -100% – +100%             | 0%      | modulazione envelope→cutoff |
| accentAmount        | float   | 0 – 100%                  | 50%     | boost volume/cutoff |
| accentThreshold     | int     | 1 – 127 (velocity)        | 100     | sopra soglia = nota accentata |
| glideTime           | float   | 0 – 2 s (skew)            | 80 ms   | 0 = glide disattivato |
| glideMode           | choice  | Always / Legato-only      | Legato-only | |
| legatoMode          | choice  | Auto / Retrigger sempre   | Auto    | governa retrigger envelope |
| pitchBendRange      | int     | 1 – 24 semitoni           | 2       | |
| masterVolume        | float   | -60 – +6 dB               | 0 dB    | smoothed |
| sampleRootNote      | int     | MIDI note 0-127           | 60 (C3) | solo modalità Sampler |
| oversamplingQuality | choice  | 1x / 2x / 4x              | 2x      | trade-off CPU/aliasing |

## Rischi tecnici / punti di attenzione

- **Aliasing oscillatore**: mitigato da PolyBLEP; verificare su note acute con test dedicato.
- **Aliasing da drive/risonanza alta**: il carattere acid implica saturazione spinta — oversampling interno nel `LadderFilter` non negoziabile.
- **Zipper noise**: ogni parametro continuo modulato da UI/automazione passa da `juce::SmoothedValue`.
- **Click su cambio switch a runtime**: `filterType` e `oscType` richiedono crossfade/mute breve, non switch istantaneo del path DSP.
- **Priorità note / legato mono**: last-note priority, coda note per release sovrapposte, retrigger vs legato secondo `legatoMode`.
- **Licensing JUCE**: verificare il piano prima del packaging per la release.
- **Pitch-shifting Sampler**: agli estremi di tastiera serve almeno interpolazione cubica per evitare artefatti.

## Piano funzionalità per milestone

**M0 — Scaffold**
1. Progetto JUCE (CMake), `PluginProcessor`/`PluginEditor` minimi, build VST3/AU caricabile.

**M1 — Core mono synth engine**
2. `ParameterLayout` completo secondo la tabella.
3. `Oscillator` (Saw PolyBLEP) + `Voice` minimale per validare il percorso audio.
4. `Envelope` (ADSR) collegata alla `Voice`.
5. `LadderFilter` con i 3 modi, risonanza/drive, oversampling.
6. `MonoVoiceManager` con last-note priority, legato/retrigger, pitch bend.
7. `GlideProcessor` integrato, rispettando `glideMode`.
8. `AccentProcessor` collegato alla velocity.
9. `SmoothedValue` su tutti i parametri continui.
10. Test DSP scritti incrementalmente insieme a ciascun modulo.

**M2 — Sampler e UI**
11. `SamplerSource` (caricamento file, pitch-tracking con interpolazione di qualità, root note) + switch click-free.
12. `AcidMoogLookAndFeel`, componenti, Panel, assemblaggio in `PluginEditor` con attachment APVTS.

**M3 — Rifinitura sonora**
13. Taratura curve resonance/drive/glide/accent, test A/B contro riferimenti 303/Moog.

**M4 — Packaging e validazione**
14. Verifica licenza JUCE.
15. Build VST3 + AU, test in Ableton e Logic, validazione `auval` e Pluginval.

## Piano UI

Stile: pannello vintage analogico scuro, fotorealistico — metallo spazzolato, manopole 3D con ombreggiatura e riflessi, viti a vista, LED colorati per gli stati.

Layout (da sinistra a destra, coerente col flusso segnale):

- **OscillatorPanel**: switch a levetta "SYNTH / SAMPLER" (click-free). In Synth nessun controllo aggiuntivo (solo la Saw, per design); in Sampler compaiono `FileDropZone`, knob Root Note e `WaveformDisplay`.
- **FilterPanel**: selettore rotativo 3 posizioni LP/HP/BP, knob grandi Cutoff e Resonance (richiamo diretto al TB-303), knob minori Drive ed Env→Filter. LED rosso all'ingresso in autooscillazione.
- **EnvelopePanel**: quattro knob allineati A/D/S/R, stile identico ma diametro minore, per gerarchia visiva.
- **PerformancePanel**: Accent Amount, Accent Threshold, Glide Time + Glide Mode, Pitch Bend Range, Master Volume; LED che lampeggia sulle note accentate.
- **Header**: logo/branding, display testuale minimale (preset fuori scope iniziale).

Principi di rendering:
- `KnobComponent` con texture pre-renderizzate ruotate via `Graphics::drawImageTransformed` — look fotorealistico senza 3D real-time.
- `AcidMoogLookAndFeel` centralizza texture, font e colori LED; i Panel restano componenti di layout.
- Uniche animazioni real-time: LED e `WaveformDisplay` (statica, aggiornata al caricamento).

## Verifica end-to-end

1. Build CMake genera VST3 + AU senza errori.
2. Ableton Live: la Saw suona, ADSR e filtro rispondono in tempo reale senza zipper noise, automazione host funziona.
3. Logic Pro: stesso test più `auval -v aumu <manufacturer> <subtype>` senza errori.
4. Sampler: drag&drop di un sample, pitch-tracking corretto su più ottave senza artefatti, root note coerente.
5. Acid: risonanza al massimo produce autooscillazione senza aliasing percepibile; accent udibile su alta velocity; glide scala col knob rispettando `glideMode`.
6. Mono: last-note priority verificata con note sovrapposte; legato vs retrigger secondo `legatoMode`; pitch bend rispetta `pitchBendRange`.
7. Cambio `filterType`/`oscType` a nota in corso senza click udibili.
8. `Tests/` DSP unit test passano.
