#include "utils/interval_model_utils.h"
#include "data_structures/interval.h"

#include "data_structures/distinct_interval_model.h"

#include <algorithm>

namespace cg::data_structures
{
    DistinctIntervalModel::DistinctIntervalModel(std::span<const Interval> intervals)
        : end(2 * intervals.size()),
          size(intervals.size())
    {
        cg::interval_model_utils::verifyEndpointsInRange(intervals);
        cg::interval_model_utils::verifyEndpointsUnique(intervals);
        cg::interval_model_utils::verifyIndicesDense(intervals);
        _leftEndpointToInterval = std::vector<std::optional<Interval>>(end);
        _rightEndpointToInterval = std::vector<std::optional<Interval>>(end);
        _indexToInterval = std::vector<Interval>();
        _indexToInterval.reserve(size);
        for(const auto& interval : intervals)
        {
            _leftEndpointToInterval[interval.Left].emplace(interval);
            _rightEndpointToInterval[interval.Right].emplace(interval);
            _indexToInterval.emplace_back(interval); // huh? I assumes indices are in order! FIX
        }

        _intervalsByIncreasingRightEndpoint = std::vector<Interval>(intervals.begin(), intervals.end());
         std::sort(_intervalsByIncreasingRightEndpoint.begin(), _intervalsByIncreasingRightEndpoint.end(),
          [](const Interval& a, const Interval& b) {
              return a.Right < b.Right;
          });  
         _intervalsByIncreasingLeftEndpoint = std::vector<Interval>(intervals.begin(), intervals.end());
         std::sort(_intervalsByIncreasingLeftEndpoint.begin(), _intervalsByIncreasingLeftEndpoint.end(),
          [](const Interval& a, const Interval& b) {
              return a.Left < b.Left;
          });

    }

    [[nodiscard]] std::optional<Interval> DistinctIntervalModel::tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const
    {
        return _rightEndpointToInterval[maybeRightEndpoint];
    }

    [[nodiscard]] std::optional<Interval> DistinctIntervalModel::tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const
    {
        return _leftEndpointToInterval[maybeLeftEndpoint];
    }

    [[nodiscard]] Interval DistinctIntervalModel::getIntervalByRightEndpoint(int rightEndpoint) const
    {
        return tryGetIntervalByRightEndpoint(rightEndpoint).value();
    }

    [[nodiscard]] Interval DistinctIntervalModel::getIntervalByLeftEndpoint(int leftEndpoint) const
    {
        return tryGetIntervalByLeftEndpoint(leftEndpoint).value();
    }

    [[nodiscard]] Interval DistinctIntervalModel::getIntervalByEndpoint(int endpoint) const
    {
        const auto& interval = tryGetIntervalByLeftEndpoint(endpoint);
        if(interval)
        {
            return interval.value();
        }
        return tryGetIntervalByRightEndpoint(endpoint).value();
    }

    [[nodiscard]] Interval DistinctIntervalModel::getIntervalByIndex(int intervalIndex) const
    {
        return _indexToInterval[intervalIndex];
    }

    [[nodiscard]] std::optional<Interval> DistinctIntervalModel::tryGetRightEndpointPredecessorInterval(int rightEndpointUpperBoundExclusive) const
    {
        auto it = std::lower_bound(
        _intervalsByIncreasingRightEndpoint.begin(),
        _intervalsByIncreasingRightEndpoint.end(),
        rightEndpointUpperBoundExclusive,
        [](const Interval& interval, int value) 
        {
            return interval.Right < value;
        });

        if (it == _intervalsByIncreasingRightEndpoint.begin())
        {
            return std::nullopt;
        }
        --it;
        return *it;
    }

    [[nodiscard]] std::optional<Interval> DistinctIntervalModel::tryGetLeftEndpointPredecessorInterval(int leftEndpointUpperBoundExclusive) const
    {
        auto it = std::lower_bound(
        _intervalsByIncreasingLeftEndpoint.begin(),
        _intervalsByIncreasingLeftEndpoint.end(),
        leftEndpointUpperBoundExclusive,
        [](const Interval& interval, int value) 
        {
            return interval.Left < value;
        });

        if (it == _intervalsByIncreasingLeftEndpoint.begin())
        {
            return std::nullopt;
        }
        --it;
        return *it;
    }
}