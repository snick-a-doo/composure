#include "random.hh"

#include <cassert>
#include <random>

std::random_device random_device;
std::mt19937 random_generator(random_device());

/// Return a random integer from low to high, inclusive with linear weighting.
int pick(int low, int high, int low_weight, int high_weight)
{
    std::vector<double> weight(high - low + 1);
    double weight_sum = 0.0;
    for (int i = low; i <= high; ++i)
    {
        weight[i - low] = low_weight + (i - low)*(high_weight - low_weight)/(high - low);
        weight_sum += weight[i - low];
    }
    std::uniform_int_distribution<> distrib(0, weight_sum - 1);
    auto r = static_cast<double>(distrib(random_generator));
    for (std::size_t i = 0; i < weight.size(); ++i)
    {
        if (r < weight[i])
            return low + i;
        r -= weight[i];
    }
    assert(false);
    return 0;
}

/// @return A random number from 0 to weights.size() - 1, weighted by the elements in
/// weights.
int pick(const std::vector<int>& weights)
{
    if (weights.size() < 2)
        return 0;
    int weight_sum = std::accumulate(weights.begin(), weights.end(), 0);
    std::uniform_int_distribution<> distrib(0, weight_sum - 1);
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
    std::vector<int> weights(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
        weights[i] = weight(pool, i);
    return pick(weights);
}
