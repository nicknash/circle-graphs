#include <vector>
#include <algorithm>
#include <ranges>
#include <format>

#include "utils/chord_model_utils.h"
#include "data_structures/chord.h"
#include "data_structures/chord_model.h"

namespace cg::utils
{
    int verifyNoGaps(std::span<const cg::data_structures::Chord> chords)
    {
        std::vector<int> endpoints;
        endpoints.reserve(2 * chords.size());
        for(const auto& c : chords)
        {
            endpoints.push_back(c.first());
            endpoints.push_back(c.second());
        }
        std::ranges::sort(endpoints);
        for(auto i = 1; i < endpoints.size(); ++i)
        {
            if(endpoints[i] > 1 + endpoints[i - 1])
            {
                throw std::invalid_argument(std::format("Chord end-points have gaps, end-point {} is never used", endpoints[i] - 1));
            }
        }
        return endpoints[endpoints.size() - 1];
    }

    cg::data_structures::ChordModel generateChordModel(int numEndpoints, std::span<int> connectionSeq)
    {
        std::vector<cg::data_structures::Chord> chords;
        for(int i = 0; i < numEndpoints; ++i)
        {
            for(auto j : connectionSeq)
            {
                cg::data_structures::Chord newChord(i, (i + j) % numEndpoints, chords.size(), 1);
                chords.push_back(newChord);
            }
        }
        cg::data_structures::ChordModel result(chords);
        return result;
    }

}