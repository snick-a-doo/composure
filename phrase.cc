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

#include "midi.hh"
#include "phrase.hh"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

using Vd = std::vector<double>;
using VNote = std::vector<Note>;

bool operator< (const Note& n1, const Note& n2)
{
    return n1.time < n2.time;
}

Phrase::Phrase(double tempo)
    : m_tempo(tempo)
{
}

Phrase::Phrase(double tempo, const VNote& notes, double end_time)
    : m_tempo(tempo),
      m_notes(notes),
      m_end_time(end_time)
{
}

std::size_t Phrase::size() const
{
    return m_notes.size();
}

const VNote& Phrase::notes() const
{
    return m_notes;
}

double Phrase::end_time() const
{
    return m_end_time;
}

double Phrase::tempo() const
{
    return m_tempo;
}

void Phrase::set_notes(double duration, double volume, const Vd& pitch, double delta_t)
{
    for (std::size_t j = 0; j < pitch.size(); j++)
        m_notes.push_back({m_end_time + j*delta_t, duration, volume, pitch[j]});
    m_end_time += duration;
}

void Phrase::append(const Phrase& phrase)
{
    m_end_time = phrase.m_end_time;
    m_notes.insert(m_notes.end(), phrase.m_notes.begin(), phrase.m_notes.end());
}

void Phrase::time_shift(double dt)
{
    for (auto& n : m_notes)
        n.time += dt;
    m_end_time += dt;
}

std::ostream& Phrase::write_midi(std::ostream& os, bool monophonic)
{
    Midi_File midi(m_tempo, 96);

    std::multiset<Note> waiting(m_notes.begin(), m_notes.end());

    auto note_off_time = [this, waiting, monophonic](const Note& note) {
        if (monophonic)
            for (auto& n : waiting)
                if (n.duration != 0.0 && n.time > note.time)
                    return n.time;
        return note.time + note.duration;
    };

    double last_time = -1.0;
    while (!waiting.empty())
    {
        const auto& note = waiting.extract(waiting.begin()).value();
        bool on = note.duration != 0.0;
        if (monophonic && on && note.time == last_time)
            continue;
        midi.add_note(note.time, on, note.pitch, note.volume);
        // Insert a note-off event.
        if (on)
        {
            waiting.emplace(note_off_time(note), 0.0, note.volume, note.pitch);
            last_time = note.time;
        }
    }
    std::cout << midi.size() << " notes, " << midi.duration() << " s" << std::endl;

    midi.write(os);
    return os;
}
