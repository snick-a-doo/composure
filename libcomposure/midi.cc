// Copyright © 2020-2021 Sam Varner
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

#include <cassert>
#include <cmath>
#include <fstream>

void write_var_be(std::ostream& os, std::uint32_t x)
{
    // Write x according to the MID standard.  x is packed into 1 to 4 bytes, 7 bits at at
    // time, most significant byte first.  The MSB is 1 for all but the last byte.
    if (x & 0xf000'0000) // Number must fit into 28 bits.
        throw Variable_Length_Value_Too_Large(x);
    if (x & 0x0fe0'0000) // Anything in the top 7 bits?
        write_be(os, std::uint8_t(0x80 | (x >> 21)));
    if (x & 0x0fff'c000) // Anything in the top 14 bits?
        write_be(os, std::uint8_t(0x80 | ((x >> 14) & 0x7f)));
    if (x & 0x0fff'ff80) // Anything in the top 21 bits?
        write_be(os, std::uint8_t(0x80 | ((x >> 7) & 0x7f)));
    return write_be(os, std::uint8_t(x & 0x7f));
}

/// Trivial type for values intended to represented as 24-bit quantities.
struct Uint24
{
    std::uint32_t x;
};

/// Write a 24-bit quantity.
template <>
void write_be(std::ostream& os, Uint24 x)
{
    write_be(os, std::uint16_t(x.x >> 8));
    write_be(os, std::uint8_t(x.x));
}

Midi_File::Midi_File(double tempo, std::uint16_t divisions)
    : m_tempo(tempo),
      m_divisions(divisions)
{
}

double Midi_File::duration() const
{
    return m_last_time;
}

std::size_t Midi_File::size() const
{
    return m_num_notes;
}

void Midi_File::add_note(double time, bool on, double pitch, double velocity)
{
    assert(time >= 0.0);
    write_var_be(m_note_buffer, std::round(m_divisions*(time - m_last_time)));
    m_last_time = time;

    if (on != m_last_note_on)
        write_be(m_note_buffer, std::uint8_t(on ? 0x90 : 0x80));
    m_last_note_on = on;

    write_be(m_note_buffer, std::uint8_t(pitch));
    write_be(m_note_buffer, std::uint8_t(velocity*127));

    if (on)
        ++m_num_notes;
}

bool Midi_File::write(std::ostream& os) const
{
    write_header(os);
    write_track(os, m_note_buffer.str().c_str(), m_note_buffer.str().size());
    return bool(os);
}

void Midi_File::write_header(std::ostream& os) const
{
    os.write("MThd", 4);
    write_be(os, std::uint32_t(6));
    write_be(os, std::uint16_t(0));
    write_be(os, std::uint16_t(1));
    write_be(os, m_divisions);
}

void Midi_File::write_track(std::ostream& os, const char* buffer, std::uint32_t size) const
{
    os.write("MTrk", 4);
    write_be(os, size + 22); // Add bytes written before and after the buffer.
    write_be(os, std::uint32_t(0x00ff5103)); // Time and "set tempo" meta-event.
    write_be(os, Uint24(60e6/m_tempo)); // µs/quarter note
    write_be(os, std::uint32_t(0x00ff5804)); // Time signature meta-event.
    write_be(os, std::uint32_t(0x04021808)); // 4/4
    os.write(buffer, size);

    // End of track
    write_be(os, std::uint32_t(0x0)); //! Timidity says the file is too short without it.
    write_be(os, std::uint16_t(0xff2f));
    write_be(os, std::uint8_t(0));
}
