#pragma once

#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalRep;
}

namespace cg::mis::shared
{
    class Naive
    {
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::SharedIntervalRep &intervals);
    };
}