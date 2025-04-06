#pragma once

namespace cg::mis::distinct
{
    class Switching
    {
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::DistinctIntervalRep &intervals);
    };
}
