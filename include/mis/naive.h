#pragma once

class SimpleIntervalRep;

namespace cg::mis::distinct
{
    class IndependentSet;

    class Naive
    {
        static void update(int endIndex, cg::mis::distinct::IndependentSet &independentSet, const cg::data_structures::DistinctIntervalRep &intervals, std::vector<int> &MIS, std::vector<int> &CMIS);
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::DistinctIntervalRep &intervals);
    };
}