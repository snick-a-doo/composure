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

#ifndef COMPOSURE_COMPOSURE_RANDOM_HH_INCLUDED
#define COMPOSURE_COMPOSURE_RANDOM_HH_INCLUDED

#include <functional>
#include <vector>

/// Set a seed for reproducible output.
void set_random_seed(unsigned int s);

/// @return A random integer from low to high, inclusive, with linear weighting.
int pick(int low, int high, int low_weight = 1, int high_weight = 1);
/// @return A random number from 0 to weights.size() - 1, weighted by the elements in
///     weights.
int pick(const std::vector<double>& weights);
/// @return A random element from pool with weights determined by the passed-in weighting
///     function.
int pick(const std::vector<double>& pool,
         std::function<double(const std::vector<double>&, std::size_t)> weight);

#endif // COMPOSURE_COMPOSURE_RANDOM_HH_INCLUDED
