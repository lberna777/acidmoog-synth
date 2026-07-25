# Plugin Bass Synth — "AcidMoog" (nome provvisorio)

## Context

Lorenzo (music producer, FL Studio/trap, in fase di miglioramento mixing) vuole progettare — non ancora implementare — un plugin VST3/AU per Ableton e Logic: un sintetizzatore di bassi ibrido tra TB-303 e Moog, con UI fotorealistica in stile hardware vintage. Il piano qui prodotto verrà passato a Fable 5 per essere rifinito ed eseguito da un agente Opus, quindi è scritto in forma tecnica e densa, non didattica.

Decisioni chiave raccolte in brainstorming:
- Framework: **JUCE (C++)**, target VST3 + AU da singolo codebase.
- Motore sonoro: **un solo oscillatore Saw** (no multi-osc, no altre waveform).
- Modalità **Sampler**: alternativa al synth engine, single-sample pitch-tracked su tutta la tastiera (root note configurabile), stessa catena filtro/envelope del synth.
- Filtro: **selezionabile LP/HP/BP**, modellato stile ladder Moog/303 con risonanza capace di autooscillare e drive/saturazione in ingresso.
- Inviluppo: ADSR classico su ampiezza (+ instradabile al filtro).
- Feature acid: **Accent** (via velocity, enfatizza volume+cutoff sulle note accentate) e **Glide/portamento** monofonico tra note legate.
- Voicing: **monofonico** (coerente con lo spirito 303/basso, glide ha senso solo mono).
- UI: stile **vintage analogico scuro** (pannello scuro, manopole metalliche 3D, viti a vista, LED), fotorealistico, non flat/vettoriale.

## Architettura del progetto (moduli e file)

Struttura JUCE standard con separazione netta DSP / stato-parametri / UI.

```
AcidMoogSynth/
├── CMakeLists.txt                     # build JUCE (o .jucer se si preferisce Projucer)
├── Source/
│   ├── PluginProcessor.h/.cpp         # AudioProcessor: entry point host, gestisce buffer audio e MIDI
│   ├── PluginEditor.h/.cpp            # AudioProcessorEditor: root della UI, monta i componenti custom
│   │
│   ├── Params/
│   │   └── ParameterLayout.h/.cpp     # APVTS: definizione unica di tutti i parametri (id, range, default, skew)
│   │
│   ├── DSP/
│   │   ├── Voice.h/.cpp               # una voce mono: oscillatore, sampler-source, envelope, filtro, glide
│   │   ├── Oscillator.h/.cpp          # Saw oscillator, band-limited (poliBLEP o simile) per evitare aliasing
│   │   ├── SamplerSource.h/.cpp       # caricamento sample, pitch-tracking su tutta la tastiera, loop/one-shot
│   │   ├── SoundSource.h              # interfaccia comune Oscillator/SamplerSource (per switch synth<->sampler)
│   │   ├── LadderFilter.h/.cpp        # filtro Moog-style 24dB/oct con switch LP/HP/BP, risonanza autooscillante, drive
│   │   ├── Envelope.h/.cpp            # ADSR (wrapper su juce::ADSR o implementazione custom se serve curve diverse)
│   │   ├── GlideProcessor.h/.cpp      # portamento mono tra note legate (glide time parametrico)
│   │   └── AccentProcessor.h/.cpp     # mapping velocity->accent: scala su volume ed eventualmente su cutoff/env
│   │
│   ├── Voicing/
│   │   └── MonoVoiceManager.h/.cpp    # gestione mono: note priority, legato/retrigger, coda note per glide
│   │
│   └── UI/
│       ├── LookAndFeel/
│       │   └── AcidMoogLookAndFeel.h/.cpp   # LookAndFeel custom: disegno manopole/switch fotorealistici
│       ├── Components/
│       │   ├── KnobComponent.h/.cpp          # slider rotativo con texture/ombre, usato per tutti i knob
│       │   ├── ToggleSwitchComponent.h/.cpp  # switch fisico (es. synth/sampler, LP/HP/BP)
│       │   ├── LedComponent.h/.cpp           # indicatori LED (accent attivo, filtro in autooscillazione, ecc.)
│       │   ├── WaveformDisplay.h/.cpp        # visualizzazione forma d'onda / sample caricato
│       │   └── FileDropZone.h/.cpp           # drag&drop del sample per la modalità Sampler
│       └── Panels/
│           ├── OscillatorPanel.h/.cpp        # sezione Saw + switch Synth/Sampler + drop zone
│           ├── FilterPanel.h/.cpp            # cutoff, resonance, drive, selettore LP/HP/BP
│           ├── EnvelopePanel.h/.cpp          # A/D/S/R
│           └── PerformancePanel.h/.cpp       # accent amount, glide time, master volume
│
├── Assets/
│   ├── Images/                        # texture pannello, sprite manopole (o vector renderizzati at-runtime)
│   └── AcidMoogSynth.png/.svg         # logo/branding pannello
│
└── Tests/
    └── DSP tests (JUCE UnitTest o Catch2): oscillatore, filtro, envelope, glide, accent
```

### Flusso dati
1. Host invia MIDI → `PluginProcessor::processBlock` → `MonoVoiceManager` risolve nota attiva/priorità/legato.
2. `MonoVoiceManager` aggiorna target pitch su `GlideProcessor`, che smussa la frequenza verso il target.
3. `Voice` legge da `SoundSource` (Oscillator o SamplerSource, in base al parametro Synth/Sampler) il segnale grezzo alla frequenza glide-corretta.
4. Segnale passa in `LadderFilter` (cutoff/risonanza modulati anche da envelope se instradato, drive applicato pre-filtro).
5. `Envelope` (ADSR) modula l'ampiezza finale; `AccentProcessor` applica boost su volume/cutoff se la nota è accentata (velocity sopra soglia o toggle esplicito).
6. Output sommato nel buffer, gestito da `PluginProcessor`.

Tutti i parametri passano da un unico `AudioProcessorValueTreeState` (APVTS) definito in `ParameterLayout`, così UI e DSP restano disaccoppiati e gli automation host funzionano out-of-the-box.

## Piano funzionalità (in ordine di dipendenza per l'implementazione)

1. **Scaffold progetto JUCE** (CMake, PluginProcessor/Editor minimi, build che carica come VST3/AU vuoto in Ableton/Logic).
2. **ParameterLayout completo**: oscType(synth/sampler placeholder), cutoff, resonance, filterType(LP/HP/BP), drive, ADSR(4), accentAmount, glideTime, masterVolume, sampleRootNote.
3. **Oscillator (Saw band-limited)** + **Voice** minimale (osc → out diretto) per validare il percorso audio.
4. **Envelope (ADSR)** collegato alla Voice.
5. **LadderFilter** con i 3 modi e risonanza/drive, integrato dopo l'oscillatore.
6. **MonoVoiceManager** con retrigger/legato reale (sostituendo eventuale gestione poliphonic di default).
7. **GlideProcessor** integrato nel voice manager.
8. **AccentProcessor** collegato a velocity in ingresso.
9. **SamplerSource** (caricamento file, pitch-tracking, root note) + switch Synth/Sampler in `SoundSource`.
10. **UI**: LookAndFeel custom e componenti (Knob/Switch/Led/WaveformDisplay/FileDropZone), poi i Panel, poi assemblaggio in `PluginEditor` con binding via `AudioProcessorValueTreeState::SliderAttachment` ecc.
11. **Rifinitura sonora**: taratura curve resonance/drive/glide per il carattere "acid" richiesto, test A/B contro riferimenti 303/Moog.
12. **Packaging**: build VST3 (Ableton) e AU (Logic), test di caricamento in entrambe le DAW, validazione con `auval` (macOS) e Pluginval.

## Piano UI (in relazione alle funzionalità sopra)

Stile: pannello vintage analogico scuro, fotorealistico — metallo spazzolato/texture scura, manopole 3D con ombreggiatura e riflessi, viti a vista agli angoli, LED colorati per stati (accent attivo, sampler loaded, filtro in autooscillazione).

Layout pannello (da sinistra a destra, coerente col flusso segnale):

- **OscillatorPanel**: switch fisico a levetta "SYNTH / SAMPLER". In modalità Synth: nessun controllo aggiuntivo (solo la Saw, per design). In modalità Sampler: appare `FileDropZone` (drag&drop) + knob "Root Note" + `WaveformDisplay` che mostra la forma caricata.
- **FilterPanel**: selettore rotativo a 3 posizioni LP/HP/BP (in stile selettore fisico, non dropdown), knob grandi per Cutoff e Resonance (il pezzo forte visivamente, richiamo diretto al TB-303), knob più piccolo per Drive. LED rosso che si accende quando la risonanza supera la soglia di autooscillazione.
- **EnvelopePanel**: quattro knob allineati A/D/S/R, stile identico ai knob filtro ma di diametro minore, per gerarchia visiva.
- **PerformancePanel**: knob Accent Amount, knob Glide Time, knob Master Volume; LED che lampeggia sulle note accentate.
- **Header**: logo/branding plugin, eventuale display testuale minimale (nome preset, se in futuro si aggiungono preset — non nello scope iniziale).

Principi di rendering:
- Manopole disegnate via `KnobComponent` con asset PNG/texture pre-renderizzate (rotazione via `Graphics::drawImageTransformed`) per ottenere il look fotorealistico senza dover fare 3D real-time — approccio standard nei plugin "hardware-style" (es. Native Instruments, Arturia).
- `AcidMoogLookAndFeel` centralizza texture di sfondo pannello, font, colori LED, così i vari Panel restano semplici componenti di layout.
- Nessun elemento decorativo dinamico costoso in CPU: le uniche animazioni real-time sono LED (accent/autooscillazione) e `WaveformDisplay` (statica, aggiornata solo al caricamento sample).

## Verifica end-to-end (una volta implementato)

1. Build CMake genera VST3 + AU senza errori.
2. Caricamento in Ableton Live: il synth suona una nota, la Saw è udibile, ADSR e filtro rispondono ai knob in tempo reale, automazione host sui parametri funziona.
3. Caricamento in Logic Pro: stesso test, più validazione `auval -v aumu <manufacturer> <subtype>` senza errori.
4. Test modalità Sampler: drag&drop di un sample proprio, verifica pitch-tracking corretto su più ottave e coerenza root note.
5. Test caratteristiche acid: risonanza portata al massimo produce autooscillazione udibile; accent su note ad alta velocity produce percepibile boost; glide tra note legate è udibile e il tempo scala col knob.
6. `Tests/` DSP unit test passano (oscillatore band-limited senza aliasing udibile, filtro stabile a risonanza massima, envelope rispetta tempi impostati).
