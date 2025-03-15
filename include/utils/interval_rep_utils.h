#pragma once

#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SimpleIntervalRep;
}

namespace cg::utils
{
    void verify_dense(std::span<const cg::data_structures::Interval> intervals);
    int compute_density(const cg::data_structures::SimpleIntervalRep& intervals);
    std::vector<cg::data_structures::Interval> generate_random_intervals(int numIntervals);
}
