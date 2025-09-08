#include <span>
#include <algorithm>
#include <ranges>
#include <vector>
#include <iostream>
#include <format>
#include <list>

#include "utils/chord_model_utils.h"
#include "data_structures/chord_model.h"
#include "data_structures/chord.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/shared_interval_model.h"

namespace cg::data_structures
{
    ChordModel::ChordModel(std::span<const Chord> chords)
    {
        _numEndpoints = 1 + cg::utils::verifyNoGaps(chords);
        _endpointToChords.resize(_numEndpoints);
        for (const auto &c : chords)
        {
            _endpointToChords[c.first()].push_back(c);
            _allChords.push_back(c);
        }
    }

    std::vector<cg::data_structures::Interval> ChordModel::cutCircle() const
    {
        std::vector<cg::data_structures::Interval> intervals;
        int endpointOffset = 0;

        // Cut the circle, giving oriented intervals
        for(auto idx = 0; idx < _numEndpoints; ++idx)
        {
            const auto& chords = _endpointToChords[idx];
            for (auto c : chords)
            {
                intervals.push_back(c.asInterval());
            }    
        }
    
        return intervals;
    }

    [[nodiscard]] DistinctIntervalModel ChordModel::toDistinctIntervalModel() const
    {
        auto intervals = std::move(cutCircle());

        std::vector<std::vector<cg::data_structures::Interval>> endpointToLeftIntervals;
        std::vector<std::vector<cg::data_structures::Interval>> endpointToRightIntervals;

        endpointToLeftIntervals.resize(_numEndpoints);
        endpointToRightIntervals.resize(_numEndpoints);

        for(const auto& interval : intervals)
        {
            endpointToLeftIntervals[interval.Left].push_back(interval);
            endpointToRightIntervals[interval.Right].push_back(interval);
        }
        for(auto& v : endpointToLeftIntervals) 
        {
            // Smallest right end-point first
            std::ranges::sort(v, [](const Interval &a, const Interval &b) {
                return a.Right < b.Right; });
        }
        for(auto& v : endpointToRightIntervals)
        {
            // Smallest left end-point first
            std::ranges::sort(v, [](const Interval &a, const Interval &b) {
                return a.Left < b.Left; });
        }

        struct Endpoint
        {
            bool isLeft;
            int intervalIdx;
        };
        std::vector<Endpoint> distinctEndpoints;

        for(auto idx = 0; idx < _numEndpoints; ++idx)
        {
            for(const auto& leftInterval : endpointToLeftIntervals[idx])
            {
                distinctEndpoints.push_back({true, leftInterval.Index});
            }
            for(const auto& rightInterval : endpointToRightIntervals[idx])
            {
                distinctEndpoints.push_back({false, rightInterval.Index});
            }
        }
        
        std::vector<int> intervalIndexToRight;
        intervalIndexToRight.resize(intervals.size());
        for(auto endpoint = 0; endpoint < distinctEndpoints.size(); ++endpoint)        
        {
            const auto& ep = distinctEndpoints[endpoint];
            if(!ep.isLeft)
            {
                intervalIndexToRight[ep.intervalIdx] = endpoint;
            }
        }
        std::vector<cg::data_structures::Interval> distinctIntervals;
        for(auto endpoint = 0; endpoint < distinctEndpoints.size(); ++endpoint)
        {
            const auto& ep = distinctEndpoints[endpoint];
            if(ep.isLeft)
            {
                int right = intervalIndexToRight[ep.intervalIdx];
                cg::data_structures::Interval interval(endpoint, right, ep.intervalIdx, 1);
                distinctIntervals.push_back(interval);
            }
        }
        auto result = cg::data_structures::DistinctIntervalModel(distinctIntervals);
        return result;
    }

    [[nodiscard]] SharedIntervalModel ChordModel::toSharedIntervalModel() const
    {
        auto intervals = std::move(cutCircle());
        auto result = cg::data_structures::SharedIntervalModel(intervals);
        return result;
    }

    [[nodiscard]] std::vector<Chord> ChordModel::getAllChords() const
    {
        return _allChords;
    }

}