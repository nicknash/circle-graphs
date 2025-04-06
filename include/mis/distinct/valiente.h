#pragma once

class SimpleIntervalRep;

namespace cg::mis::distinct
{
    class Valiente
    {
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::DistinctIntervalRep& intervals);
    };
}
