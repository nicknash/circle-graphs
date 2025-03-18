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
    void verifyEndpointsDense(std::span<const cg::data_structures::Interval> intervals);
    void verifyIndicesDense(std::span<const cg::data_structures::Interval> intervals);
    int computeDensity(const cg::data_structures::SimpleIntervalRep& intervals);
    std::vector<cg::data_structures::Interval> generateRandomIntervals(int numIntervals);
}
