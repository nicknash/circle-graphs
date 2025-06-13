#include <vector>
#include <list>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/distinct/valiente.h"

namespace cg::mis::distinct
{
    std::vector<cg::data_structures::Interval> Valiente::computeMIS(const cg::data_structures::DistinctIntervalModel& intervals)
    {
        std::vector<int> MIS(intervals.end + 1, 0);
        std::vector<int> CMIS(intervals.size, 0);

        cg::mis::IndependentSet result(intervals.size);

        for(auto i = 0; i < intervals.end; ++i)
        {
            auto maybeOuterInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeOuterInterval)
            {
                auto outerInterval = maybeOuterInterval.value();
                for(auto j = outerInterval.Right; j > outerInterval.Left; --j)
                {
                    auto maybeInnerInterval = intervals.tryGetIntervalByLeftEndpoint(j);
                    MIS[j] = MIS[j + 1];
                    result.setSameNextInterval(j);
                    if(maybeInnerInterval)
                    {
                        auto innerInterval = maybeInnerInterval.value();
                        auto candidate = MIS[innerInterval.Right + 1] + CMIS[innerInterval.Index];
                        if(innerInterval.Right < outerInterval.Right && // Strictly speaking, this bounds check could be removed because CMIS and MIS on the previous line
                                                                        // will both be zero when it is false, but it's a bit confusing to write the code that way. 
                           candidate > MIS[j + 1])
                        {
                            MIS[j] = candidate;
                            result.setNewNextInterval(j, innerInterval);
                        }
                    }
                }
                CMIS[outerInterval.Index] = outerInterval.Weight + MIS[outerInterval.Left + 1];
                result.assembleContainedIndependentSet(outerInterval);
            }
        }

        
        for(auto i = intervals.end - 1; i >= 0; --i)
        {
            auto maybeInterval = intervals.tryGetIntervalByLeftEndpoint(i);
            result.setSameNextInterval(i);
            
            MIS[i] = MIS[i + 1];
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = MIS[interval.Right + 1] + CMIS[interval.Index];
                if(candidate > MIS[i + 1])
                {
                    MIS[i] = candidate;
                    result.setNewNextInterval(i, interval);
                }
            }
        }

        const auto& intervalsInMis = result.buildIndependentSet(MIS[0]); 
        
        return intervalsInMis;
    }
}
