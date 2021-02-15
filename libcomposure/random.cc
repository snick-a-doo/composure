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

#include <cassert>
#include <random>
#include <stdexcept>

std::random_device random_device;
std::mt19937 random_generator(random_device());

class Bad_Weights : public std::runtime_error
{
public:
    Bad_Weights()
        : std::runtime_error("One or more weights, all positive, are required.")
    {}
};

void set_random_seed(unsigned int s)
{
    random_generator.seed(s);
}

int pick(int low, int high, int low_weight, int high_weight)
{
    std::vector<double> pool(high - low + 1);
    std::iota(pool.begin(), pool.end(), low);
    return pick(pool, [=](const auto&, std::size_t i) { return low_weight +
                (i - low)*double(high_weight - low_weight)/(high - low); });
}

int pick(const std::vector<double>& weights)
{
    if (weights.empty()
        || std::any_of(weights.begin(), weights.end(), [](int x) { return x < 0; })
        || std::all_of(weights.begin(), weights.end(), [](int x) { return x == 0; }))
        throw Bad_Weights();

    int weight_sum = std::accumulate(weights.begin(), weights.end(), 0);
    std::uniform_real_distribution<> distrib(0, weight_sum);
    auto r = distrib(random_generator);
    for (std::size_t i = 0; i < weights.size(); ++i)
    {
        if (r < weights[i])
            return i;
        r -= weights[i];
    }
    assert(false);
    return 0;
}

int pick(const std::vector<double>& pool,
         std::function<double(const std::vector<double>&, std::size_t)> weight)
{
    std::vector<double> weights(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
        weights[i] = weight(pool, i);
    return pool[pick(weights)];
}
