#include "midi.hh"

#include <cmath>
#include <fstream>

void write_var_be(std::ostream& os, std::uint32_t x)
{
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

/// Trivial time for values intended to represented as 24-bit quantities.
struct Uint24
{
    std::uint32_t x;
};

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

void Midi_File::add_note(double time, bool on, double pitch, double volume)
{
    write_var_be(m_note_buffer, std::round(m_divisions*(time - m_last_time)));
    m_last_time = time;

    if (on != m_last_note_on)
        write_be(m_note_buffer, std::uint8_t(on ? 0x90 : 0x80));
    m_last_note_on = on;

    write_be(m_note_buffer, std::uint8_t(pitch));
    write_be(m_note_buffer, std::uint8_t(volume*127));

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
    write_be(os, size + 48);
    write_be(os, std::uint32_t(0x00ff5103)); // Time and "set tempo" meta-event.
    write_be(os, Uint24(60e6/m_tempo)); // µs/quarter note
    os.write(buffer, size);

    // End of track
    write_be(os, std::uint32_t(0x0));
    write_be(os, std::uint16_t(0xff2f));
    write_be(os, std::uint8_t(0));
}
