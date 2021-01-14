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

#include "doctest.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

bool bits(std::ostringstream& os, std::vector<std::uint8_t> xs)
{
    bool ok = os.str() == std::string(xs.begin(), xs.end());
    if (!ok)
    {
        for (auto s : os.str())
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << int(std::uint8_t(s)) << ' ';
        std::cout << std::endl;
        for (auto x : xs)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << int(x) << ' ';
        std::cout << std::endl;
    }
    return ok;
}

TEST_CASE("big endian")
{
    std::ostringstream os;
    SUBCASE("8-bit")
    {
        write_be(os, std::uint8_t(21));
        write_be(os, std::uint8_t(0));
        write_be(os, std::uint8_t(255));
        CHECK(bits(os, {21, 0, 255}));
    }
    SUBCASE("16-bit")
    {
        write_be(os, std::uint16_t(0xa1));
        write_be(os, std::uint16_t(0));
        write_be(os, std::uint16_t(0xb1c1));
        CHECK(bits(os, {0,0xa1, 0,0, 0xb1,0xc1}));
    }
    SUBCASE("32-bit")
    {
        write_be(os, std::uint32_t(0));
        write_be(os, std::uint32_t(0xa1));
        write_be(os, std::uint32_t(0xa1b1));
        write_be(os, std::uint32_t(0xa1b1c1));
        write_be(os, std::uint32_t(0xa1b1c1d1));
        CHECK(bits(os, {0,0,0,0, 0,0,0,0xa1, 0,0,0xa1,0xb1,
                        0,0xa1,0xb1,0xc1, 0xa1,0xb1,0xc1,0xd1}));
    }
}

TEST_CASE("variable length")
{
    std::ostringstream os;
    SUBCASE("8-bit")
    {
        write_var_be(os, 0);
        write_var_be(os, 127);
        CHECK(bits(os, {0, 0x7f}));
    }
    SUBCASE("16-bit")
    {
        write_var_be(os, 128);
        write_var_be(os, 255);
        write_var_be(os, 0x1a1b);
        write_var_be(os, 0x1a9b);
        CHECK(bits(os, {0x81,0, 0x81,0x7f, 0xb4,0x1b, 0xb5,0x1b}));
    }
    SUBCASE("24-bit")
    {
        write_var_be(os, 0x4000);
        write_var_be(os, 0x1a1b1c);
        // 00011010 00011011 00011100
        //  1101000  0110110  0011100
        // 11101000 10110110 00011100
        write_var_be(os, 0x1fffff);
        CHECK(bits(os, {0x81,0x80,0x00,
                        0xe8,0xb6,0x1c,
                        0xff,0xff,0x7f}));
    }
    SUBCASE("32-bit")
    {
        write_var_be(os, 0x200000);
        write_var_be(os, 0x0a1b1c1d);
        // 00001010 00011011 00011100 00011101
        //  1010000  1101100  0111000  0011101
        // 11010000 11101100 10111000 00011101
        write_var_be(os, 0x0fffffff);
        CHECK(bits(os, {0x81,0x80,0x80,0,
                        0xd0,0xec,0xb8,0x1d,
                        0xff,0xff,0xff,0x7f}));
    }
    SUBCASE("too big")
    {
        CHECK_THROWS_AS(write_var_be(os, 0x10000000), Variable_Length_Value_Too_Large);
        CHECK_THROWS_AS(write_var_be(os, 0x1a1b1c1d), Variable_Length_Value_Too_Large);
        CHECK_THROWS_AS(write_var_be(os, 0xffffffff), Variable_Length_Value_Too_Large);
    }
}
