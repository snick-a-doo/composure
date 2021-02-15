// Copyright © 2020 Sam Varner
//
// This file is part of Composure.
//
// Composure is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// Composure is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with Composure.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef COMPOSURE_COMPOSURE_PHRASE_HH_INCLUDED
#define COMPOSURE_COMPOSURE_PHRASE_HH_INCLUDED

#include <iosfwd>
#include <string>
#include <vector>

/// A single musical note.
struct Note
{
    double time; ///< The time when the note starts, in beats.
    double duration; ///< beats
    double volume; ///< 0 to 1
    double pitch;  ///< MIDI note: 60 + (half steps from middle C).
    int generation; ///< Incremented for each "compose" pass.
};

/// A sequence of notes, possibly overlapping in time.
class Phrase
{
    using VNote = std::vector<Note>;

public:
    Phrase(double tempo);

    /// Generate notes and add them to a Phrase.
    /// @param phrase A set of notes to add to, using the notes at the end as a starting
    ///     point.  If phrase is empty, copies of tonic are used as the starting point.
    /// @param tonic A the MIDI note number for the tonic of the key.  60 is middle C, 61 is a
    ///     half step higher, etc.  All generated notes are in the key.
    /// @param voices The number of voices in the generated phrase.
    /// @param max_range Note generation stops when the number of half steps between the
    ///    highest and lowest note in the voices exceeds this value.
    /// @return A Phrase with generated notes.
    void compose(int tonic, int voices, int max_range, bool chromatic);

    /// A fitness function determines points of interest in the Phrase.  Sections that start
    /// at those points are extracted, spliced and returned.  The sections may overlap.
    void edit();

    /// Append notes to the end of the phrase.
    void append_notes(const VNote& notes);
    /// Read-only note access.
    const VNote& notes() const;

    /// Write the phrase to a file in MIDI format.
    std::ostream& write_midi(std::ostream& os, bool monophonic);

private:
    double m_tempo; ///< Tempo in beat/min.
    VNote m_notes; ///< The notes in the composition so far.
    std::string m_log; ///< Messages for the log file.
    int m_generation = 0; ///< A count of the number of calls to compose()>
};

#endif // COMPOSURE_COMPOSURE_PHRASE_HH_INCLUDED
