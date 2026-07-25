#pragma once

#include "../Params/ParamTypes.h"

#include <vector>

/** Gestione delle note per una voce monofonica con last-note priority.

    Tiene lo stack delle note premute: l'ultima vince, e rilasciandola si torna a
    suonare quella sotto (comportamento atteso da chi suona bassi acid a dita).
    Decide anche se una nota e' legato — cioe' e' arrivata mentre un'altra era
    ancora premuta — informazione che serve sia al glide sia al retrigger
    dell'envelope.
*/
class MonoVoiceManager
{
public:
    struct Action
    {
        enum class Type { None, Start, Release };

        Type type { Type::None };
        int note { 60 };
        int velocity { 100 };
        bool legato { false };      // suonata sopra una nota gia' premuta
        bool retrigger { true };    // l'envelope deve ripartire da zero
    };

    void setLegatoMode (LegatoMode m) noexcept { legatoMode = m; }

    Action noteOn (int midiNote, int velocity);
    Action noteOff (int midiNote);
    void allNotesOff();

    bool hasActiveNote() const noexcept { return ! heldNotes.empty(); }
    int getActiveNote() const noexcept;

private:
    struct HeldNote
    {
        int note;
        int velocity;
    };

    std::vector<HeldNote> heldNotes;
    LegatoMode legatoMode { LegatoMode::Auto };
};
