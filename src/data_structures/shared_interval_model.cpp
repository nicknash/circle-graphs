#include "utils/interval_model_utils.h"
#include "data_structures/interval.h"

#include "data_structures/shared_interval_model.h"

#include <algorithm>
#include <ranges>

namespace cg::data_structures
{
    SharedIntervalModel::SharedIntervalModel(std::span<const Interval> intervals) : 
    size(intervals.size()),
    end(cg::interval_model_utils::getMaxRightEndpoint(intervals) + 1)
    {
        cg::interval_model_utils::verifyEndpointsInRange(intervals);
        cg::interval_model_utils::verifyIndicesDense(intervals);

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


    [[nodiscard]] std::vector<Interval> SharedIntervalModel::getAllIntervalsWithLeftEndpoint(int leftEndpoint) const
    {
        return _leftEndpointToIntervals[leftEndpoint];
    }

    [[nodiscard]] std::vector<Interval> SharedIntervalModel::getAllIntervalsWithRightEndpoint(int rightEndpoint) const
    {
        return _rightEndpointToIntervals[rightEndpoint];
    }
}