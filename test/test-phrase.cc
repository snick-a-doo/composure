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

#include <phrase.hh>

#include "doctest.h"

bool operator== (const Note& n1, const Note& n2)
{
    return n1.time == n2.time
        && n1.duration == n2.duration
        && n1.volume == n2.volume
        && n1.pitch == n2.pitch;
}

TEST_CASE("notes")
{
    Phrase phrase(60);
    Note n1(1.2, 2.3, 0.8, 33);
    Note n2(4.5, 6.7, 0.2, 44);
    SUBCASE("empty")
    {
        CHECK(phrase.notes().empty());
    }
    SUBCASE("ascending")
    {
        phrase.append_notes({n1, n2});
        const auto& n = phrase.notes();
        CHECK(n.size() == 2);
        CHECK(n[0] == n1);
        CHECK(n[1] == n2);
    }
    SUBCASE("descending")
    {
        phrase.append_notes({n2, n1});
        const auto& n = phrase.notes();
        CHECK(n.size() == 2);
        CHECK(n[0] == n1);
        CHECK(n[1] == n2);
    }
}
