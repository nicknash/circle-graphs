#pragma once

#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalRep
    {
        std::vector<std::vector<Interval>> _leftEndpointToIntervals;
        

    public:
        const int end;
        const int size;
        SharedIntervalRep(std::span<Interval> intervals);

        [[nodiscard]] std::vector<Interval> getAllIntervalsWithLeftEndpoint(int leftEndpoint) const;
        [[nodiscard]] Interval getIntervalByIndex(int intervalIndex) const;
    };
}