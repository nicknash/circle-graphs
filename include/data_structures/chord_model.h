#pragma once

#include <span>
#include <vector>
#include "utils/chord_model_utils.h"

namespace cg::data_structures
{
    class DistinctIntervalModel;
    class SharedIntervalModel;
    class Chord;
    class Interval;

    class ChordModel
    {
        int _numEndpoints;
        std::vector<Chord> _allChords;
        std::vector<std::vector<Chord>> _endpointToChords; 
        std::vector<Interval> cutCircle() const;
    public:
        ChordModel(std::span<const Chord> chords);
        [[nodiscard]] DistinctIntervalModel toDistinctIntervalModel() const;
        [[nodiscard]] SharedIntervalModel toSharedIntervalModel() const;
        [[nodiscard]] std::vector<Chord> getAllChords() const;
    };
}