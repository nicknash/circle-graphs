#include "data_structures/interval.h"
#include "utils/interval_model_utils.h"

#include "mis/implicit_independent_set.h"

#include <format>
#include <stdexcept>

namespace cg::mis
{
    ImplicitIndependentSet::ImplicitIndependentSet(int maxNumIntervals) // should accept a max interval end-point really instead
    {
        _intervalIndexToDirectlyContained.resize(maxNumIntervals);
        _endpointToRange.emplace(-1, Range{-2,-1, cg::data_structures::Interval(-2, -1, 0, 0)});
        _endpointToRange.emplace(2*maxNumIntervals+1, Range{2*maxNumIntervals,2*maxNumIntervals+1, cg::data_structures::Interval(2*maxNumIntervals, 2*maxNumIntervals+1, 0, 0)});
    }

    void ImplicitIndependentSet::setRange(int left, int right, const cg::data_structures::Interval& interval) 
    {        
        
        auto pred = _endpointToRange.upper_bound(right);
        --pred;
        auto predRange = pred->second; 
        if(predRange.left < left) 
        {
            // PPPPP
            //   LLLLL
            pred->second.right = left - 1;
            _endpointToRange.erase(pred);
            _endpointToRange.emplace(left - 1, Range{predRange.left,left - 1, predRange.interval});
        }
        else //if(predRange.left == left)
        {
            // PPPPP
            // LLLLLL
            _endpointToRange.erase(pred);
        }
        /*else
        {

            //   PPPPP
            // LLLLLL
            throw std::runtime_error("SURPRISING");

        }*/

        auto next = _endpointToRange.upper_bound(right);
        auto nextRange = next->second;
        if(right < nextRange.right)
        {
            //   NNNNNN
            // RRRRRR
            next->second.left = right + 1;
        }
        else if(right == nextRange.right)
        {
            //  NNNNN
            // RRRRRR
            _endpointToRange.erase(next);
        }
        else
        {
            // NNNNN
            // RRRRRRR
            throw std::runtime_error("SURPRISING2");
        }


        auto [it, wasInserted] = _endpointToRange.emplace(right, Range{left,right, interval});
    
        if(!wasInserted)
        {
                      throw std::runtime_error("bug!");
        }
    }

    // It's worth a quick explanation of the space complexity implied by calling assembleContainedIndependentSet for each of k intervals.
    // This results in, for each of the k intervals, the set of intervals directly contained in that interval being stored. Directly contained in an interval I, means intervals that are contained 
    // in I, but not in any other interval that is contained in I.
    //
    // This requires, for all the k intervals only O(k) space. The reason is that for any interval, it can only be directly contained in at most 2 other intervals (because, if there was a third
    // interval containing any interval, it would have to contain one of the other two, violating direct containment)
    void ImplicitIndependentSet::assembleContainedIndependentSet(const cg::data_structures::Interval &interval)
    {

        auto maybeNext = _endpointToRange.upper_bound(interval.Left);
      
        auto &containedSet = _intervalIndexToDirectlyContained[interval.Index];
        auto last = _endpointToRange.end();
        --last;
        while (maybeNext != last)
        {
            auto rangeHere = maybeNext->second;
            if(rangeHere.right < interval.Left || rangeHere.left > interval.Right)
            {
                break;
            }

            auto intervalHere = rangeHere.interval;
            
            containedSet.push_front(intervalHere);
            maybeNext = _endpointToRange.upper_bound(intervalHere.Right);
        }
    }

    std::vector<cg::data_structures::Interval> ImplicitIndependentSet::buildIndependentSet(int expectedCardinality)
    {
        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(expectedCardinality);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(expectedCardinality);
        auto maybeInterval = _endpointToRange.begin();
        ++maybeInterval; // skip sentinel.

        auto last = _endpointToRange.end();
        --last; // skip sentinel.
        while(maybeInterval != last)
        {
            const auto& interval = maybeInterval->second.interval;
            pendingIntervals.push_back(interval);
            while(!pendingIntervals.empty())
            {
                const auto& newInterval = pendingIntervals.back();
                pendingIntervals.pop_back();
                intervalsInMis.push_back(newInterval);
                const auto& allContained = _intervalIndexToDirectlyContained[newInterval.Index];
                for(auto c : allContained)
                {   
                    pendingIntervals.push_back(c);
                }
            }
            maybeInterval = _endpointToRange.upper_bound(interval.Right);
        }
        (void)expectedCardinality;
        cg::interval_model_utils::verifyNoOverlaps(intervalsInMis);
        return intervalsInMis;
    }
}