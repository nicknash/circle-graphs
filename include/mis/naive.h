#pragma once

class SimpleIntervalRep;

namespace cg::mis
{
    class Naive
    {
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::SimpleIntervalRep &intervals);
    };
}