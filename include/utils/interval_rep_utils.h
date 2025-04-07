#pragma once

#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class DistinctIntervalRep;
}

namespace cg::utils
{
    void verifyEndpointsInRange(std::span<const cg::data_structures::Interval> intervals);
    void verifyEndpointsUnique(std::span<const cg::data_structures::Interval> intervals);
    void verifyIndicesDense(std::span<const cg::data_structures::Interval> intervals);
    void verifyNoOverlaps(std::span<const cg::data_structures::Interval> intervals);
    int computeDensity(const cg::data_structures::DistinctIntervalRep& intervals);
    std::vector<cg::data_structures::Interval> generateRandomIntervals(int numIntervals, int seed);
}
