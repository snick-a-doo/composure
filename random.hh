#include <functional>
#include <vector>

int pick(int low, int high, int low_weight = 1, int high_weight = 1);
int pick(const std::vector<int>& weights);
int pick(const std::vector<double>& pool,
         std::function<double(const std::vector<double>&, std::size_t)> weight);
