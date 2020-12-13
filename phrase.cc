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

std::string Phrase::score(const Vd& partials, double tempo) const
{
    std::ostringstream out;
    out << "f1 0 4096 10";
    for (const auto& p : partials)
        out << ' ' << p;
    out << std::endl << score(tempo);
    return out.str();
}

std::string Phrase::score(double tempo) const
{
    std::ostringstream out;
    if (tempo > 0.0)
        out << "t 0 " << tempo << std::endl;
    for (const auto& note : m_notes)
        out << "i1 "
              << note.time << ' '
              << note.duration << ' '
              << note.volume << ' '
              << note.pitch << std::endl;
    return out.str();
}

void Phrase::time_shift(double dt)
{
    for (auto& n : m_notes)
        n.time += dt;
    m_end_time += dt;
}

void Phrase::write_midi(const std::string& file_name)
{
    Midi_File midi(m_tempo, 96);

    std::multiset<Note> waiting(m_notes.begin(), m_notes.end());
    while (!waiting.empty())
    {
        const auto& note = waiting.extract(waiting.begin()).value();
        bool on = note.duration != 0.0;
        midi.add_note(note.time, on, note.pitch, note.volume);
        // Insert a note-off event.
        if (on)
            waiting.emplace(note.time + note.duration, 0.0, note.volume, note.pitch);
    }
    std::cout << midi.size() << " notes, " << midi.duration() << " s" << std::endl;

    std::ofstream file(file_name);
    midi.write(file);
}
