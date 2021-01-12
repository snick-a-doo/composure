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
#include "random.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>

using Vd = std::vector<double>;
using VNote = std::vector<Note>;

namespace
{
    /// Half steps from tonic for a major scale.
    const std::vector<double> major_scale = {0, 2, 4, 5, 7, 9, 11};
    const std::vector<double> chromatic_scale = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    /// @return The difference between the largest and smallest elements of v.
    double range(const Vd& v)
    {
        auto [min, max] = std::minmax_element(v.begin(), v.end());
        return *max - *min;
    }

    /// @param key A MIDI note number of the tonic of the key.
    /// @param pitch A MIDI note number of a note.
    /// @return The degree of the scale (0-6) of pitch in key, or no value if pitch is not in
    /// the key.
    std::optional<int> degree(const auto& scale, double key, double pitch)
    {
        while (pitch < key)
            pitch += 12;
        auto halfs = std::fmod(pitch - key, 12.0);
        assert(0 <= halfs && halfs < 12);
        auto it = std::find(scale.begin(), scale.end(), halfs);
        if (it == scale.end())
            return std::nullopt;
        return static_cast<int>(std::distance(scale.begin(), it));
    }

    /// @return The pitch delta degrees up or down the scale from a given pitch.
    double scale_pitch(const auto& scale, double key, double pitch, double delta)
    {
        int octave = std::trunc((pitch - key)/12);
        auto deg = degree(scale, key, pitch);
        assert(deg);
        *deg += delta;
        while (*deg >= static_cast<int>(scale.size()))
        {
            ++octave;
            *deg -= scale.size();
        }
        while (*deg < 0)
        {
            --octave;
            *deg += scale.size();
        }
        assert(0 <= *deg && *deg < static_cast<int>(scale.size()));
        double out = key + 12*octave + scale[*deg];
        assert(out >= pitch || delta< 0);
        return out;
    }

    /// @return Interval halved a given number of times.
    double subdivide(double interval, int times)
    {
        return interval/std::pow(2, times);
    }

    /// @return A measure of the dissonance between two integral pitches.
    int dissonance(int p1, int p2)
    {
        // Disfavor unison.  Octaves are handled in the switch.
        if (p1 == p2)
            return 12;
        switch (std::abs(p1 - p2) % 12)
        {
        case 1:
        case 2:
        case 6:
        case 10:
        case 11:
            return 8;
        case 5:
        case 7:
            return 4;
        case 0:
        case 3:
        case 4:
        case 8:
        case 9:
            return 1;
        default:
            assert(false);
        }
    }

    /// @return A measure of the dissonance in a chord.
    double discord(const std::deque<Note>& notes)
    {
        double sum = 0.0;
        for (std::size_t i = 0; i < notes.size(); ++i)
            for (std::size_t j = 0; j < i; ++j)
                sum += dissonance(static_cast<int>(notes[i].pitch),
                                  static_cast<int>(notes[j].pitch));
        return sum;
    }

    int weight_discord(const Vd& pitch, std::size_t i)
    {
        int sum = 0;
        for (size_t j = 0; j < pitch.size(); ++j)
            if (j != i)
                sum += dissonance(static_cast<int>(pitch[i]),
                                  static_cast<int>(pitch[j]));
        return sum;
    }
}

bool operator< (const Note& n1, const Note& n2)
{
    return n1.time < n2.time;
}

Phrase::Phrase(double tempo)
    : m_tempo(tempo)
{
}

void Phrase::compose(int tonic, int voices, int max_range, bool chromatic)
{
    // Initialize pitch with the last notes in the phrase.  Note that the last notes
    // aren't necessarily latest in time.
    Vd pitch(voices, tonic);
    if (m_notes.size() >= static_cast<size_t>(voices))
        for (int i = 0; i < voices; ++i)
            pitch[i] = m_notes[m_notes.size() - voices + i].pitch;

    // The number of repetitions is used as a weight for choosing which note to move.  The
    // last element means "none of the above".  It's the most likely until we accumulate
    // some repetitions.
    Vd repetitions(voices + 1, 0.0);
    repetitions.back() = voices;

    // Add notes to the phrase until the range exceeds max_range, or we get 1000 notes.
    double end_time = m_notes.empty() ? 0.0 : m_notes.back().time + m_notes.back().duration;
    for (double span = 0.0; span < max_range && m_notes.size() < 1000; span = range(pitch))
    {
        // Move the "most discordant" note by -2, -1, 0, 1, or 2 scale degrees.
        std::vector<double> discord(pitch.size());
        std::transform(pitch.begin(), pitch.end(), discord.begin(),
                       [&pitch](auto x) { return weight_discord(pitch, x); });
        auto move_idx = pick(discord);
        pitch[move_idx] = scale_pitch(chromatic ? chromatic_scale : major_scale,
                                      tonic, pitch[move_idx],
                                      chromatic ? pick(-4, 4) : pick(-2, 2));

        // Possibly move a random note weighted by age.
        auto old_idx = pick(repetitions);
        if (old_idx != voices)
            pitch[old_idx] = scale_pitch(chromatic ? chromatic_scale : major_scale,
                                         tonic, pitch[old_idx], 2*pick(0, 1) - 1);

        // Pick a random duration for the note: 1/4, 1/8, 1/16.  Favor shorter notes when
        // the pitch range is large.
        double dur = subdivide(4.0, pick(0, 2, max_range - span, span));
        auto delta_t = dur*(voices+1)/voices;
        if (end_time > pitch.size()*delta_t)
            for (std::size_t j = 0; j < pitch.size(); j++)
                m_notes.emplace_back(end_time - j*delta_t, dur, 0.8, pitch[j], m_generation);
        end_time += dur;

        // Update the ages of notes that weren't changed.
        for (int i = 0; i < voices; ++i)
            ++repetitions[i];
        repetitions[move_idx] = 0;
        if (old_idx != voices)
            repetitions[old_idx] = 0;
    }

    std::sort(m_notes.begin(), m_notes.end());
    ++m_generation;
}

void Phrase::edit()
{
    if (m_notes.empty())
        return;

    auto points_of_interest = [](Vd& in) {
        std::vector<std::size_t> ps;
        auto [min, max] = std::minmax_element(in.begin(), in.end() - in.size()/2);
        if (min == in.end() || max == in.end())
            return ps;
        auto thresh = (*min + *max)/2.0;
        bool over = false;
        for (std::size_t i = 1; i < in.size(); ++i)
        {
            if (in[i] < thresh)
                over = false;
            else if (!over && in[i] < in[i-1])
            {
                ps.push_back(i);
                over = true;
            }
        }
        return ps;
    };

    // Make a running consonance measure.
    Vd time, pitch, cons;
    std::deque<Note> notes;
    const std::size_t bin = std::min(m_notes.size()/4, std::size_t(36)); //!! param
    for (const auto& n : m_notes)
    {
        notes.push_back(n);
        if (notes.size() < bin)
            continue;
        time.push_back(n.time);
        pitch.push_back(n.pitch);
        cons.push_back(-discord(notes));
        notes.pop_front();
    }

    auto poi = points_of_interest(cons);
    VNote edited;
    double end_time = 0.0;
    for (std::size_t ip = 0; ip < poi.size(); ++ip)
    {
        double dt = end_time - m_notes[poi[ip]].time;
        for (std::size_t jp = 0; jp < bin; ++jp)
        {
            edited.push_back(m_notes[poi[ip] + jp]);
            edited.back().time += dt;
        }
        end_time = edited.back().time;
    }
    m_notes = edited;
}

const VNote& Phrase::notes() const
{
    return m_notes;
}

void Phrase::append_notes(const VNote& notes)
{
    m_notes.insert(m_notes.end(), notes.begin(), notes.end());
    std::sort(m_notes.begin(), m_notes.end());
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
        if (on)
        {
            // Insert a note-off event.
            waiting.emplace(note_off_time(note), 0.0, note.volume, note.pitch);
            last_time = note.time;
        }
    }

    midi.write(os);
    return os;
}
