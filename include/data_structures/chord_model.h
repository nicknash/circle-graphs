#pragma once

#include <span>
#include <vector>
#include "utils/chord_model_utils.h"

namespace cg::data_structures
{
    class DistinctIntervalModel;
    class SharedIntervalModel;
    class Chord;

    class ChordModel
    {
        std::vector<std::vector<Chord>> _endpointToChords; 
    public:
        ChordModel(std::span<const Chord> chords);
        [[nodiscard]] DistinctIntervalModel toDistinctIntervalModel() const;
        [[nodiscard]] SharedIntervalModel toSharedIntervalModel() const;
    };
}