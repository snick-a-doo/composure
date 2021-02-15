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

#include "random.hh"
#include "doctest.h"

#include <iostream>

bool close(const double& actual, const double& expected, double tol)
{
    if (std::abs(expected - actual) < tol)
        return true;
    std::cout << actual << " != " << expected << " ±" << tol << std::endl;
    return false;
}

TEST_CASE("empty weights")
{
    set_random_seed(0u);
    CHECK_THROWS(pick({}));
    CHECK_THROWS(pick({0}));
    CHECK_THROWS(pick({-9}));
    CHECK_THROWS(pick({0, 0, 0}));
    CHECK_THROWS(pick({1, -1, 1}));
    CHECK(pick({3}) == 0);
    for (int i = 0; i < 10; ++i)
        CHECK(pick({0, 3, 0}) == 1);
}

TEST_CASE("pick weights")
{
    set_random_seed(0u);
    std::vector<double> weights({1.0, 3.0, 0.0, 6.0});
    std::vector<int> picks(weights.size());
    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
        ++picks[pick(weights)];
    // Expect weight*N/Σweight = 100*weight
    CHECK(close(picks[0], 100, 10));
    CHECK(close(picks[1], 300, 30));
    CHECK(picks[2] == 0);
    CHECK(close(picks[3], 600, 30));
}

TEST_CASE("pick range")
{
    set_random_seed(0u);
    std::vector<int> picks(4);
    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
        ++picks[pick(10, 13) - 10];
    for (int p : picks)
        CHECK(close(p, 250, 25));
}

TEST_CASE("pick negative range")
{
    set_random_seed(0u);
    std::vector<int> picks(5);
    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
        ++picks[pick(-2, 2) + 2];
    for (int p : picks)
        CHECK(close(p, 200, 30));
}
