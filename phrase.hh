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

#ifndef COMPOSE_COMPOSE_PHRASE_HH_INCLUDED
#define COMPOSE_COMPOSE_PHRASE_HH_INCLUDED

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
    using Vd = std::vector<double>;
    using VNote = std::vector<Note>;

public:
    Phrase() = default;
    Phrase(const VNote& notes, double end_time);

    /// Add notes to the phrase
    /// @param dt The time between the start of each note.  May be negative.
    void set_notes(double duration, double volume, const Vd& pitch, double dt);
    void time_shift(double dt);
    void append(const Phrase& phrase);

    /// @return A Csound score string.
    std::string score(double tempo = 0.0) const;
    /// Include a sine table in the Csound score.  Useful if required by the Csound
    /// orchestra file.
    std::string score(const Vd& partials, double tempo = 0.0) const;

    /// @return The number of notes in the phrase.
    std::size_t size() const;
    /// Read-only note access.
    const VNote& notes() const;
    /// @return The time at the end of the phrase.
    double end_time() const;

private:
    VNote m_notes;
    double m_end_time = 0.0;
};

#endif // COMPOSE_COMPOSE_PHRASE_HH_INCLUDED
