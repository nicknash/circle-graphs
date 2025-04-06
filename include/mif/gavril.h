#pragma once

#include <span>

#include "data_structures/simple_interval_rep.h"

namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mif
{
    class Gavril
    {
    public:
        static void computeMif(std::span<const Interval> intervals)
        {
            // sort intervals by length.
            auto levelEndIndex = 123; // index one past end of shortest intervals
            
            auto levelNumber = 0;

            cg::data_structures::DistinctIntervalRep intervalRep(intervals);
            while(levelEndIndex < intervals.size())
            {
                for(auto x = 0; x < intervalRep.end; x++)
                {
                    for(auto y = x + 1; y < intervals.)
                }

                ++levelNumber;
                // advance levelEndIndex to one past end of next length category
            }
        }
    };
}