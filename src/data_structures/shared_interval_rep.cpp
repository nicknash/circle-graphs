#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/shared_interval_rep.h"

#include <algorithm>
#include <ranges>

namespace cg::data_structures
{
    SharedIntervalRep::SharedIntervalRep(std::span<const Interval> intervals) : 
    size(intervals.size()),
    end(cg::utils::getMaxRightEndpoint(intervals) + 1)
    {
        cg::utils::verifyEndpointsInRange(intervals);
        cg::utils::verifyIndicesDense(intervals);

        _leftEndpointToIntervals = std::vector<std::vector<Interval>>(end);
        _rightEndpointToIntervals = std::vector<std::vector<Interval>>(end);
        for(const auto& interval : intervals)
        {
            _leftEndpointToIntervals[interval.Left].push_back(interval);
            _rightEndpointToIntervals[interval.Right].push_back(interval);
        }
        for(auto i = 0; i < end; ++i)
        {
            auto& rightIntervals = _rightEndpointToIntervals[i];
            std::ranges::sort(rightIntervals, [](const Interval &a, const Interval &b) {
                return a.length() > b.length(); });
 
            auto& leftIntervals = _leftEndpointToIntervals[i];
            std::ranges::sort(leftIntervals, [](const Interval &a, const Interval &b) {
                return a.length() > b.length(); });
 
        }
    }


    [[nodiscard]] std::vector<Interval> SharedIntervalRep::getAllIntervalsWithLeftEndpoint(int leftEndpoint) const
    {
        return _leftEndpointToIntervals[leftEndpoint];
    }

    [[nodiscard]] std::vector<Interval> SharedIntervalRep::getAllIntervalsWithRightEndpoint(int rightEndpoint) const
    {
        return _rightEndpointToIntervals[rightEndpoint];
    }
}