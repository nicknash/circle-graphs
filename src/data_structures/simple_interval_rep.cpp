#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/simple_interval_rep.h"

namespace cg::data_structures
{
    SimpleIntervalRep::SimpleIntervalRep(std::span<const Interval> intervals)
        : end(2 * intervals.size()),
          size(intervals.size())
    {
        cg::utils::verifyEndpointsDense(intervals);
        cg::utils::verifyIndicesDense(intervals);
        _leftEndpointToInterval = std::vector<std::optional<Interval>>(end);
        _rightEndpointToInterval = std::vector<std::optional<Interval>>(end);
        _indexToInterval = std::vector<Interval>();
        _indexToInterval.reserve(size);
        for(const auto& interval : intervals)
        {
            _leftEndpointToInterval[interval.Left].emplace(interval.Left, interval.Right, interval.Index);
            _rightEndpointToInterval[interval.Right].emplace(interval.Left, interval.Right, interval.Index);
            _indexToInterval.emplace_back(interval.Left, interval.Right, interval.Index);
        }
    }

    [[nodiscard]] std::optional<Interval> SimpleIntervalRep::tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const
    {
        return _rightEndpointToInterval[maybeRightEndpoint];
    }

    [[nodiscard]] std::optional<Interval> SimpleIntervalRep::tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const
    {
        return _leftEndpointToInterval[maybeLeftEndpoint];
    }

    [[nodiscard]] Interval SimpleIntervalRep::getIntervalByRightEndpoint(int rightEndpoint) const
    {
        return tryGetIntervalByRightEndpoint(rightEndpoint).value();
    }

    [[nodiscard]] Interval SimpleIntervalRep::getIntervalByLeftEndpoint(int leftEndpoint) const
    {
        return tryGetIntervalByLeftEndpoint(leftEndpoint).value();
    }

    [[nodiscard]] Interval SimpleIntervalRep::getIntervalByEndpoint(int endpoint) const
    {
        const auto& interval = tryGetIntervalByLeftEndpoint(endpoint); 
        if(interval)
        {
            return interval.value();
        }
        return tryGetIntervalByLeftEndpoint(endpoint).value();
    }

    [[nodiscard]] Interval SimpleIntervalRep::getIntervalByIndex(int intervalIndex) const
    {
        return _indexToInterval[intervalIndex];
    }
}