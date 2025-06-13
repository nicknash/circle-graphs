#pragma once

#include <span>

#include "data_structures/distinct_interval_model.h"

namespace cg::data_structures
{
    class DistinctIntervalModel;
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