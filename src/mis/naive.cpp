#include <vector>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"

#include "mis/naive.h"

namespace cg::mis
{
    void Naive::computeMIS(const cg::data_structures::SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.End + 1, 0);
        std::vector<int> CMIS(intervals.Size, 0);
        for(auto i = 0; i < intervals.End; ++i)
        {
            auto maybeIntervalRight = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeIntervalRight)
            {
                auto interval = maybeIntervalRight.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
            }
            for (auto j = i - 1; j >= 0; --j)
            {
                MIS[j] = MIS[j + 1];
                auto maybeIntervalLeft = intervals.tryGetIntervalByLeftEndpoint(j);
                if(maybeIntervalLeft)
                {
                    auto interval = maybeIntervalLeft.value();
                    if(interval.Right <= i)
                    {
                        MIS[j] = std::max(MIS[j + 1], 1 + CMIS[interval.Index] + MIS[interval.Right + 1]);
                    }
                } 
            }
        }
    }
}