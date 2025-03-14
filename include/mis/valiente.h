#pragma once

class SimpleIntervalRep;

namespace cg::mis
{
    class Valiente
    {
    public:
        static void computeMIS(const cg::data_structures::SimpleIntervalRep& intervals);
    };
}
