#pragma once

class SimpleIntervalRep;

namespace cg::mis
{
    class Naive
    {
    public:
        static void computeMIS(const cg::data_structures::SimpleIntervalRep &intervals);
    };
}