#pragma once

#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalRep
    {
    public:
        SharedIntervalRep(std::span<const Interval> intervals);

        [[nodiscard]] std::vector<Interval> getAllIntervalsWithLeftEndpoint(int leftEndpoint) const;
        [[nodiscard]] Interval getIntervalByIndex(int intervalIndex) const;
    };
}