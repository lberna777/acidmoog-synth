#include "MonoVoiceManager.h"

#include <algorithm>

int MonoVoiceManager::getActiveNote() const noexcept
{
    return heldNotes.empty() ? -1 : heldNotes.back().note;
}

MonoVoiceManager::Action MonoVoiceManager::noteOn (int midiNote, int velocity)
{
    const auto wasPlaying = ! heldNotes.empty();

    // Ripremere una nota gia' nello stack la porta in cima invece di duplicarla.
    heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                                     [midiNote] (const HeldNote& h) { return h.note == midiNote; }),
                     heldNotes.end());

    heldNotes.push_back ({ midiNote, velocity });

    Action action;
    action.type = Action::Type::Start;
    action.note = midiNote;
    action.velocity = velocity;
    action.legato = wasPlaying;
    action.retrigger = (legatoMode == LegatoMode::AlwaysRetrigger) || ! wasPlaying;

    return action;
}

MonoVoiceManager::Action MonoVoiceManager::noteOff (int midiNote)
{
    const auto wasActive = (! heldNotes.empty() && heldNotes.back().note == midiNote);

    heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                                     [midiNote] (const HeldNote& h) { return h.note == midiNote; }),
                     heldNotes.end());

    Action action;

    if (heldNotes.empty())
    {
        action.type = Action::Type::Release;
        action.note = midiNote;
        return action;
    }

    // Restano note premute: se abbiamo rilasciato quella che stava suonando, si
    // torna alla precedente in legato (envelope mantenuto).
    if (! wasActive)
        return action;   // nota rilasciata non attiva: nessun cambiamento udibile

    const auto& next = heldNotes.back();

    action.type = Action::Type::Start;
    action.note = next.note;
    action.velocity = next.velocity;
    action.legato = true;
    action.retrigger = (legatoMode == LegatoMode::AlwaysRetrigger);

    return action;
}

void MonoVoiceManager::allNotesOff()
{
    heldNotes.clear();
}
