#pragma once

namespace cg::mis
{
    class Switching
    {
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::SimpleIntervalRep &intervals);
    };
}
