#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/simple_interval_rep.h"

namespace cg::data_structures
{
    SimpleIntervalRep::SimpleIntervalRep(std::span<const Interval> intervals)
        : End(2 * intervals.size()),
          Size(intervals.size())
    {
        cg::utils::verify_dense(intervals);
        _leftEndpointToInterval = std::vector<std::optional<Interval>>(End);
        _rightEndpointToInterval = std::vector<std::optional<Interval>>(End);
        for(const auto& interval : intervals)
        {
            _leftEndpointToInterval[interval.Left].emplace(interval.Left, interval.Right, interval.Index);
            _rightEndpointToInterval[interval.Right].emplace(interval.Left, interval.Right, interval.Index);
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
}