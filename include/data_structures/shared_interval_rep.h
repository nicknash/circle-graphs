#pragma once

#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalModel
    {
        std::vector<std::vector<Interval>> _leftEndpointToIntervals;
        std::vector<std::vector<Interval>> _rightEndpointToIntervals;
    public:
        const int end;
        const int size;
        SharedIntervalModel(std::span<const Interval> intervals);

        [[nodiscard]] std::vector<Interval> getAllIntervalsWithLeftEndpoint(int leftEndpoint) const;
        [[nodiscard]] std::vector<Interval> getAllIntervalsWithRightEndpoint(int rightEndpoint) const;
        [[nodiscard]] Interval getIntervalByIndex(int intervalIndex) const;
    };
}