#include <string>
#include <vector>

/// A single musical note.
struct Note
{
    double time; ///< The time when the note starts, in beats.
    double duration; ///< beats
    double volume; ///< 0 to 1
    double pitch;  ///< MIDI note: 60 + (half steps from middle C).
};

/// A sequence of notes, possibly overlapping in time.
struct Phrase
{
    std::vector<Note> notes;
    double end_time = 0.0;
};

using Vd = std::vector<double>;

/// Add notes to the score.
/// @param t The start time for the first note.
/// @param period The duration of each note.
/// @param pitch A vector of the midi notes to add.
/// @param dt The time between the start of each note.  If negative, notes start
///    start before t.
/// return@ The start time for the next note (t + duration).
Phrase set_notes(Phrase phrase, double duration, double volume, const Vd& pitch, double dt = 0.0);
Phrase walk(Phrase phrase, int tonic, int voices, int max_range);
std::vector<Phrase> filter(Phrase phrase);

std::string score(const Phrase& phrase, double tempo = 0.0);
/// Include a sine table in the Csound score.  Useful if required by the Csound
/// orchestra file.
std::string score(const Phrase& phrase, const Vd& partials, double tempo = 0.0);

Phrase time_shift(Phrase phrase, double dt);
