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

#ifndef COMPOSURE_COMPOSURE_MIDI_HH_INCLUDED
#define COMPOSURE_COMPOSURE_MIDI_HH_INCLUDED

#include <bit>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <stdexcept>

/// Exception thrown if there's an attempt to write a value with more than 28 bits as a
/// variable-length quantity.
class Variable_Length_Value_Too_Large : public std::runtime_error
{
public:
    Variable_Length_Value_Too_Large(std::uint32_t x)
        : std::runtime_error("Variable length values must fit in 28 bits: " + std::to_string(x))
    {}
};

/// Write a fixed-length value most significant first.
template <typename T>
void write_be(std::ostream& os, T x)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        os.write((char*)&x, sizeof x);
        return;
    }
    T y = 0;
    for (std::size_t i = 0; i < sizeof x; ++i)
    {
        y = (y << 8) + (x & 0xff);
        x = x >> 8;
    }
    os.write((char*)&y, sizeof y);
}

/// Write a variable-length value as 1 to 4 bytes, most significant first.
/// @param x The value to write.  Must fit in 28 bits or throws
/// Variable_Length_Value_Too_Large.
void write_var_be(std::ostream& os, std::uint32_t x);

/// A class to accumulate notes and write a single-track MIDI file.
class Midi_File
{
public:
    /// @param tempo The number beats per second.
    /// @param divisions The number of ticks in a beat.
    Midi_File(double tempo, std::uint16_t divisions);
    /// Add a single note-on or note-off event.
    /// @param delta_t Time since the last event, possibly zero.
    /// @param on True for note-on events, false for note-off.
    /// @param pitch The MIDI note number of the pitch.  Middle C is 60.
    /// @param velocity The velocity factor: 0 to 1.
    void add_note(double delta_t, bool on, double pitch, double velocity);
    /// Write the MIDI file to a stream.
    bool write(std::ostream& os) const;

    /// @return The total length of the performance.
    double duration() const;
    /// @return The number of notes in the performance.
    std::size_t size() const;

private:
    /// Write the MIDI file header chunk.
    void write_header(std::ostream& os) const;
    /// Write the MIDI file track checkn.
    void write_track(std::ostream& os, const char* buffer, std::uint32_t size) const;

    double m_tempo; ///< Quarter beats per second.
    std::uint16_t m_divisions; ///< The number of ticks in a beat.
    std::ostringstream m_note_buffer;
    bool m_last_note_on = false;
    double m_last_time = 0.0;
    std::size_t m_num_notes = 0;
};

#endif // COMPOSURE_COMPOSURE_MIDI_HH_INCLUDED
