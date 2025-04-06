#pragma once

#include <span>

#include "data_structures/distinct_interval_rep.h"

namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mif
{
    class Gavril
    {
    public:
        static void computeMif(std::span<const cg::data_structures::Interval> intervals);
    };
}