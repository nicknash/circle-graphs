#pragma once

#include <span>
#include <utility>  

namespace cg::data_structures
{
    class Chord;
    class ChordModel;
}

namespace cg::utils
{
    int verifyNoGaps(std::span<const cg::data_structures::Chord> chords);
    cg::data_structures::ChordModel generateChordModel(int numEndpoints, std::span<int> connectionSeq);
}