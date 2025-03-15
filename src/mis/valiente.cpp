#include <vector>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"

#include "mis/valiente.h"

namespace cg::mis
{
    void Valiente::computeMIS(const cg::data_structures::SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.End + 1, 0);
        std::vector<int> CMIS(intervals.Size, 0);
        for(auto i = 0; i < intervals.End; ++i)
        {
            auto maybeOuterInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeOuterInterval)
            {
                auto outerInterval = maybeOuterInterval.value();
                for(auto j = outerInterval.Right - 1; j > outerInterval.Left; --j)
                {
                    auto maybeInnerInterval = intervals.tryGetIntervalByLeftEndpoint(j);
                    if(maybeInnerInterval)
                    {
                        auto innerInterval = maybeInnerInterval.value();
                        auto candidate = MIS[innerInterval.Right + 1] + CMIS[innerInterval.Index];
                        if(innerInterval.Right < outerInterval.Right && // Strictly speaking, this bounds check could be removed because CMIS and MIS on the previous line
                                                                        // will both be zero when it is false, but it's a bit confusing to write the code that way. 
                           candidate > MIS[innerInterval.Left + 1])
                        {
                            MIS[j] = candidate;
                        }
                    }
                    else
                    {
                        MIS[j] = MIS[j + 1];
                    }
                }
                CMIS[outerInterval.Index] = 1 + MIS[outerInterval.Left + 1];
            }
        }
        for(auto i = intervals.End - 1; i >= 0; --i)
        {
            auto maybeInterval = intervals.tryGetIntervalByLeftEndpoint(i);
            
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                MIS[i] = std::max(MIS[i + 1], 1 + MIS[interval.Right + 1] + CMIS[interval.Index]);
            }
            else
            {
                MIS[i] = MIS[i + 1];
            }
        }
    }
}
