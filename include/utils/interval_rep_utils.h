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
    std::vector<cg::data_structures::Interval> generateRandomIntervalsShared(int numIntervals, int maxPerEndpoint, int maxLength, int seed);
    int getMaxRightEndpoint(std::span<const cg::data_structures::Interval> intervals);
    long sumWeights(std::span<cg::data_structures::Interval> intervals);
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponents(cg::data_structures::DistinctIntervalRep& intervalRep);
}
