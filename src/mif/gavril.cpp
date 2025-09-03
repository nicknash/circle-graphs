
#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

namespace cg::mif
{
    static void computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        // No, we actually want the containment DAG here.

        // sort intervals by length.
        auto levelEndIndex = 123; // index one past end of shortest intervals

        auto levelNumber = 0;

        cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        while (levelEndIndex < intervals.size())
        {
            auto levelStartIndex = 3343;

            for (auto intervalIdx = levelStartIndex; intervalIdx < levelEndIndex; ++intervalIdx)
            {
                const auto& w_interval = intervals[intervalIdx];


                // may not want these loops: ? does the "The right endpoints r_w .. " begin to describe the mechanism for the computation of FR_w,i FL_w,i ?
                for (auto x = 0; x < intervalModel.end; x++)
                {
                    for (auto y = x + 1; y < intervalModel.end; ++y)
                    {
                        // if l_w < x and r_w \in [x, y]
                        if(w_interval.Left < x && x <= w_interval.Right && w_interval.Right <= y)
                        {
                            // Construct FR_w,i[x, y] (note we call i levelNumber) 

                        }

                        // if y < r_w and l_w \in [x, y] 
                        if(y < w_interval.Right && x <= w_interval.Left && w_interval.Left <= y)
                        {
                            // Construct FL_w,i[x,y] (note we call i levelNumber)
                        }
                    
                    }
                }
            }

            ++levelNumber;
            // advance levelEndIndex to one past end of next length category
        }
    }
}