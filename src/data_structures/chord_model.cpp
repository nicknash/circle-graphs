#include <span>
#include <vector>

#include "utils/chord_model_utils.h"
#include "data_structures/chord_model.h"
#include "data_structures/chord.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/shared_interval_model.h"

namespace cg::data_structures
{
    ChordModel::ChordModel(std::span<const Chord> chords)
    {
        auto maxEndpoint = cg::utils::verifyNoGaps(chords);
        _endpointToChords.reserve(maxEndpoint + 1);
        for (const auto &c : chords)
        {
            _endpointToChords[c.first()].push_back(c);
        }
    }

    [[nodiscard]] DistinctIntervalModel ChordModel::toDistinctIntervalModel() const
    {
    }

    [[nodiscard]] SharedIntervalModel ChordModel::toSharedIntervalModel() const
    {
    }

}