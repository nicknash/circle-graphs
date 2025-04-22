#pragma once

#include <optional>
#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class DistinctIntervalRep
    {
        std::vector<std::optional<Interval>> _leftEndpointToInterval; 
        std::vector<std::optional<Interval>> _rightEndpointToInterval; 
        std::vector<Interval> _indexToInterval;
        std::vector<Interval> _intervalsByIncreasingRightEndpoint;
    public:
        const int end; 
        const int size;

        DistinctIntervalRep(std::span<const Interval> intervals);

        [[nodiscard]] std::optional<Interval> tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const;
        [[nodiscard]] std::optional<Interval> tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const;

        [[nodiscard]] std::optional<Interval> tryGetRightEndpointPredecessorInterval(int rightEndpointUpperBoundExclusive) const; // find rightmost interval with right end-point < rightEndpointUpperBoundExclusive

        [[nodiscard]] Interval getIntervalByRightEndpoint(int rightEndpoint) const;
        [[nodiscard]] Interval getIntervalByLeftEndpoint(int leftEndpoint) const;
        [[nodiscard]] Interval getIntervalByEndpoint(int endpoint) const;
        [[nodiscard]] Interval getIntervalByIndex(int intervalIndex) const;
    };
}