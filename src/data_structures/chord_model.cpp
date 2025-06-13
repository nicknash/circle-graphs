#include <span>
#include <algorithm>
#include <ranges>
#include <vector>

#include "utils/chord_model_utils.h"
#include "data_structures/chord_model.h"
#include "data_structures/chord.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/shared_interval_model.h"
#include "data_structures/interval.h"

namespace cg::data_structures
{
    ChordModel::ChordModel(std::span<const Chord> chords)
    {
        _numEndpoints = 1 + cg::utils::verifyNoGaps(chords);
        _endpointToChords.reserve(_numEndpoints);
        for (const auto &c : chords)
        {
            _endpointToChords[c.first()].push_back(c);
        }
        for(auto i = 0; i < _numEndpoints; ++i)
        {
            auto& chords = _endpointToChords[i];
            std::ranges::sort(chords, [](const Chord &a, const Chord &b) {
                return a.second() < b.second(); });
        }
    }

    [[nodiscard]] DistinctIntervalModel ChordModel::toDistinctIntervalModel() const
    {
        std::vector<cg::data_structures::Interval> intervals;
        int targetIdx = 0;
        int endpointOffset = 0;
        int numIntervals = 0;
        for(auto idx = 0; idx < _numEndpoints; ++idx)
        {
            const auto& chords = _endpointToChords[idx];
            const auto& left = idx + endpointOffset;
            auto iter = chords.begin();
            const Interval interval(idx + endpointOffset, iter->second() + endpointOffset, numIntervals, 1);
            intervals.push_back(interval);
            ++numIntervals;
            ++iter;
            while(iter != chords.end())
            {
                const Interval interval(idx + endpointOffset, iter->second() + endpointOffset, numIntervals, 1);
                intervals.push_back(interval);
                ++iter;
                ++numIntervals;
                ++endpointOffset;
            }    
        }
        auto result = cg::data_structures::DistinctIntervalModel(intervals);
        return result;
    }

    [[nodiscard]] SharedIntervalModel ChordModel::toSharedIntervalModel() const
    {
    }

}