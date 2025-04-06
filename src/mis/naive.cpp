#include <vector>
#include <list>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/naive.h"

namespace cg::mis
{
    void Naive::update(int i, cg::mis::IndependentSet &independentSet, const cg::data_structures::DistinctIntervalRep &intervals, std::vector<int> &MIS, std::vector<int> &CMIS)
    {
        for (auto j = i - 1; j >= 0; --j)
        {
            MIS[j] = MIS[j + 1];
            independentSet.setSameNextRightEndpoint(j);
            auto maybeInterval = intervals.tryGetIntervalByLeftEndpoint(j);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                if (interval.Right <= i)
                {
                    auto candidate = 1 + CMIS[interval.Index] + MIS[interval.Right + 1];
                    if (candidate > MIS[j + 1])
                    {
                        independentSet.setNewNextRightEndpoint(j, interval.Right);
                        MIS[j] = candidate;
                    }
                }
            }
        }
    }

    std::vector<cg::data_structures::Interval> Naive::computeMIS(const cg::data_structures::DistinctIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.end + 1, 0);
        std::vector<int> CMIS(intervals.size, 0);
        
        IndependentSet independentSet(intervals);

        for(auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                independentSet.assembleContainedIndependentSet(interval);
            }
            update(i, independentSet, intervals, MIS, CMIS);
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet();
        return intervalsInMis;
    }
}

