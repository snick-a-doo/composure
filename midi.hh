#include <bit>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <stdexcept>

class Variable_Length_Value_Too_Large : public std::runtime_error
{
public:
    Variable_Length_Value_Too_Large(std::uint32_t x)
        : std::runtime_error("Variable length values must fit in 28 bits: " + std::to_string(x))
    {}
};

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

void write_var_be(std::ostream& os, std::uint32_t x);

class Midi_File
{
public:
    Midi_File(double tempo, std::uint16_t divisions);
    void add_note(double delta_t, bool on, double pitch, double volume);
    bool write(std::ostream& os) const;

    double duration() const;
    std::size_t size() const;
private:
    void write_header(std::ostream& os) const;
    void write_track(std::ostream& os, const char* buffer, std::uint32_t size) const;

    double m_tempo;
    std::uint16_t m_divisions;
    std::ostringstream m_note_buffer;
    bool m_last_note_on = false;
    double m_last_time = 0.0;
    std::size_t m_num_notes = 0;
};
