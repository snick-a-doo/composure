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

#include "composer.hh"
#include "random.hh"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>

using Vd = std::vector<double>;
using Vi = std::vector<int>;

namespace
{
    constexpr int n_degrees = 7;
    /// Half steps from tonic for a major scale.
    constexpr std::array<double, n_degrees> major = {0, 2, 4, 5, 7, 9, 11};

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
    std::optional<int> degree(double key, double pitch)
    {
        while (pitch < key)
            pitch += 12;
        auto halfs = std::fmod(pitch - key, 12.0);
        assert(0 <= halfs && halfs < 12);
        auto it = std::find(major.begin(), major.end(), halfs);
        if (it == major.end())
            return std::nullopt;
        return static_cast<int>(std::distance(major.begin(), it));
    }

    /// @return The pitch delta degrees up or down the scale from a given pitch.
    double scale(double key, double pitch, double delta)
    {
        int octave = std::trunc((pitch - key)/12);
        auto deg = degree(key, pitch);
        assert(deg);
        *deg += delta;
        while (*deg >= n_degrees)
        {
            ++octave;
            *deg -= n_degrees;
        }
        while (*deg < 0)
        {
            --octave;
            *deg += n_degrees;
        }
        assert(0 <= *deg && *deg < static_cast<int>(major.size()));
        double out = key + 12*octave + major[*deg];
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

Phrase compose(const Phrase& phrase, int tonic, int voices, int max_range)
{
    // Initialize pitch with the last notes in the phrase.  Note that the last notes
    // aren't necessarily latest in time.
    Vd pitch(voices, tonic);
    if (phrase.size() >= static_cast<size_t>(voices))
        for (int i = 0; i < voices; ++i)
            pitch[i] = phrase.notes()[phrase.size() - voices + i].pitch;

    // The number of repetitions is used as a weight for choosing which note to move.  The
    // last element means "none of the above".  It's the most likely until we accumulate
    // some repetitions.
    Vd repetitions(voices + 1, 0.0);
    repetitions.back() = voices;

    Phrase out = phrase;
    // Add notes to the phrase until the range exceeds max_range, or we get 1000 notes.
    for (double span = 0.0; span < max_range && out.size() < 1000; span = range(pitch))
    {
        // Move the "most discordant" note by -2, -1, 0, 1, or 2 scale degrees.
        std::vector<double> discord(pitch.size());
        std::transform(pitch.begin(), pitch.end(), discord.begin(),
                       [&pitch](auto x) { return weight_discord(pitch, x); });
        auto move_idx = pick(discord);
        pitch[move_idx] = scale(tonic, pitch[move_idx], pick(-2, 2));

        // Possibly move a random note weighted by age.
        auto old_idx = pick(repetitions);
        if (old_idx != voices)
            pitch[old_idx] = scale(tonic, pitch[old_idx], 2*pick(0, 1) - 1);

        // Pick a random duration for the note: 1/4, 1/8, 1/16.  Favor shorter notes when
        // the pitch range is large.
        double dur = subdivide(4.0, pick(0, 2, max_range - span, span));
        // Put successive arpeggio voices in the past.
        out.set_notes(dur, 0.1, pitch, -dur*(voices+1)/voices);

        // Update the ages of notes that weren't changed.
        for (int i = 0; i < voices; ++i)
            ++repetitions[i];
        repetitions[move_idx] = 0;
        if (old_idx != voices)
            repetitions[old_idx] = 0;
    }
    return out;
}

Phrase edit(const Phrase& phrase)
{
    auto points_of_interest = [](Vd& in) {
        std::vector<std::size_t> ps;
        auto [min, max] = std::minmax_element(in.begin(), in.end() - in.size()/2);
        if (min == in.end() || max == in.end())
            return ps;
        auto thresh = (*min + *max)/2.0;
        bool found = false;
        for (std::size_t i = 1; i < in.size(); ++i)
        {
            if (in[i] < thresh)
                found = false;
            else if (!found && in[i] < in[i-1])
            {
                ps.push_back(i);
                found = true;
            }
        }
        return ps;
    };

    // Sort notes by time.
    std::vector<Note> sorted_notes = phrase.notes();
    std::sort(sorted_notes.begin(), sorted_notes.end(),
              [](const Note& n1, const Note& n2) { return n1.time < n2.time; });
    // Make a running consonance measure.
    Vd time;
    Vd pitch;
    Vd cons;
    std::deque<Note> notes;
    const std::size_t bin = std::min(phrase.size()/4, std::size_t(36)); //!! param
    for (const auto& n : sorted_notes)
    {
        notes.push_back(n);
        if (notes.size() < bin)
            continue;
        time.push_back(n.time);
        pitch.push_back(n.pitch);
        cons.push_back(-discord(notes));
        notes.pop_front();
    }

    std::vector<std::size_t> poi = points_of_interest(cons);
    Phrase edited;
    for (std::size_t ip = 0; ip < poi.size(); ++ip)
    {
        std::vector<Note> part;
        for (std::size_t jp = 0; jp < bin; ++jp)
            part.push_back(sorted_notes[poi[ip] + jp]);
        Phrase p = Phrase(part, sorted_notes[poi[ip] + bin].time);
        p.time_shift(edited.end_time() - p.notes().front().time);
        edited.append(p);
    }
    return edited;
}
