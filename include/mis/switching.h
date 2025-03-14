#pragma once

namespace cg::data_structures
{
    class SimpleIntervalRep;
}

namespace cg::mis
{
    class Switching
    {
    public:
        static void computeMIS(const cg::data_structures::SimpleIntervalRep &intervals);
    };
}
