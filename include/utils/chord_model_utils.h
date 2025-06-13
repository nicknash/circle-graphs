#pragma once

#include <span>

namespace cg::data_structures
{
    class Chord;
}

namespace cg::utils
{
    int verifyNoGaps(std::span<const cg::data_structures::Chord> chords);
}