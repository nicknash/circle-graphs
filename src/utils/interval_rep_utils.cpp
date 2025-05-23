#include <format>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>
#include <ranges>
#include <random>
#include <tuple>
#include <limits>
#include <map>
#include <stack>
#include <set>

    #include <iostream>


#include "data_structures/interval.h"
#include "data_structures/distinct_interval_rep.h"

#include "utils/interval_rep_utils.h"
 
 namespace cg::utils
 {
    void verifyEndpointsInRange(std::span<const cg::data_structures::Interval> intervals)
    {
        auto end = 2 * intervals.size();
        std::vector<bool> alreadySeen(end, false);   
        int maxRight = std::numeric_limits<int>::min();     
        for(const auto& i : intervals)
        {
            if(i.Left < 0)
            {
                throw std::invalid_argument(std::format("Invalid left end-point {} for {}", i.Left, i));
            }
            if(i.Right >= end)
            {
                throw std::invalid_argument(std::format("Invalid right end-point {} for {}", i.Right, i));
            }
            alreadySeen[i.Left] = alreadySeen[i.Right] = true;
            if(i.Right > maxRight)
            {
                maxRight = i.Right;
            }
        }
        for(auto i = 0; i < maxRight; ++i)
        {
            if(!alreadySeen[i])
            {
                throw std::invalid_argument(std::format("Interval end-points have gaps, end-point {} is never used.", i));
            }
        }
    }

    void verifyEndpointsUnique(std::span<const cg::data_structures::Interval> intervals)
    {
        auto end = 2 * intervals.size();
        std::vector<bool> alreadySeen(end, false);        
        for(const auto& i : intervals)
        {
            if(alreadySeen[i.Left])
            {
                throw std::invalid_argument(std::format("Left end-point of interval {} is used by more than one interval, end-points must be unique", i));
            }
            if(alreadySeen[i.Right])
            {
                throw std::invalid_argument(std::format("Right end-point of interval {} is used by more than one interval, end-points must be unique", i));
            }
            alreadySeen[i.Left] = true;
            alreadySeen[i.Right] = true;
        }
    }

    void verifyIndicesDense(std::span<const cg::data_structures::Interval> intervals)
    {
        std::vector<bool> alreadySeen(intervals.size(), false);        
        for(const auto& i : intervals)
        {
            if(i.Index < 0 || i.Index >= intervals.size())
            {
                throw std::invalid_argument(std::format("Invalid index {} for interval {}", i.Index, i));
            }
            if(alreadySeen[i.Index])
            {
                throw std::invalid_argument(std::format("Interval index {} is used by more than one interval, interval indices must be unique.", i.Index));
            }
            alreadySeen[i.Index] = true;
        }
        for(auto i = 0; i < alreadySeen.size(); ++i)
        {
            if(!alreadySeen[i])
            {
                throw std::invalid_argument(std::format("Interval indices have gaps, index {} is never used.", i));
            }
        }
    }

    void verifyNoOverlaps(std::span<const cg::data_structures::Interval> intervals)
    {
        struct Event
        {
            int point;                                     // Endpoint (either Left or Right)
            bool isOpening;                                // true = opening, false = closing
            const cg::data_structures::Interval *interval; // Pointer to the original interval
        };
        std::vector<Event> events;
        events.reserve(2 * intervals.size());
        // Create events for each interval
        for (const auto &interval : intervals)
        {
            events.push_back({interval.Left, true, &interval});   // Opening event
            events.push_back({interval.Right, false, &interval}); // Closing event
        }

        // Sort events:
        //  - Primary sort by point
        //  - Secondary: closing events before opening events if points are equal
        std::ranges::sort(events, [](const Event &a, const Event &b)
                          {
            if (a.point == b.point) 
            {
                throw std::runtime_error(std::format("Overlap detected: Equal end-points encountered {} and {} for intervals {} and {}", a.point, b.point, *a.interval, *b.interval));
            }
            return a.point < b.point; });

        // Sweep line logic using std::set for active start points
        std::set<int> activeStartPoints; // Tracks only starting points

        for (const auto &event : events)
        {
            if (event.isOpening)
            {
                activeStartPoints.insert(event.interval->Left); // Track opening point
            }
            else
            {
                // On closing event: ensure no "later" opening point is still active
                const auto &closingInterval = *event.interval;

                // Remove the interval's starting point since it's now closed
                activeStartPoints.erase(closingInterval.Left);

                // Check if any interval that started later is still active
                auto it = activeStartPoints.upper_bound(closingInterval.Left); // First "later" starting point
                if (it != activeStartPoints.end())
                {
                    throw std::runtime_error(std::format("Overlap detected between interval opening at {} and interval {}", (*it), closingInterval));
                }
            }
        }
    }

    int computeDensity(const cg::data_structures::DistinctIntervalRep& intervals)
    {
        auto numOpen = 0;
        auto maxOpen = 0;
        for(auto i = 0; i < intervals.end; ++i)
        {
            if(intervals.tryGetIntervalByLeftEndpoint(i))
            {
                ++numOpen;
            }
            else
            {
                --numOpen;
            }
            if(numOpen > maxOpen)
            {
                maxOpen = numOpen;
            }
        }        
        return maxOpen;
    }

    // Note that these correspond to the interval graphs studied in:
    // SCHEINERMAN, E. R. 1988. Random interval graphs. Combinatorica 8, 4, 357–371.  
    std::vector<cg::data_structures::Interval> generateRandomIntervals(int numIntervals, int seed)
    {
        std::vector<int> endPoints(2 * numIntervals);
        std::iota(endPoints.begin(), endPoints.end(), 0);
        std::mt19937 rng(seed);
        std::shuffle(endPoints.begin(), endPoints.end(), rng);

        std::vector<cg::data_structures::Interval> result;
        result.reserve(numIntervals);
        for(auto i = 0; i < numIntervals; ++i)
        {
            auto firstIndex = 2 * i;
            int leftEndpoint;
            int rightEndpoint;
            const auto weight = 1;//std::uniform_int_distribution<>(1, 10 * numIntervals)(rng);

            if(endPoints[firstIndex] < endPoints[firstIndex + 1])
            {
                leftEndpoint = endPoints[firstIndex];
                rightEndpoint = endPoints[firstIndex + 1];
            }
            else
            {
                leftEndpoint = endPoints[firstIndex + 1];
                rightEndpoint = endPoints[firstIndex];
            }
            result.emplace_back(leftEndpoint, rightEndpoint, i, weight);
        }
        return result;
    }

    // Note that these correspond to the interval graphs studied in:
    // S CHEINERMAN, E. R. 1990. An evolution of interval graphs. Discrete Math. 82, 3, 287–302.
    std::vector<cg::data_structures::Interval> generate_random_intervals_with_radius(int numIntervals)
    {
        throw std::exception();
        // Generate random centre point in [0, 1] and random radius in [0, R]
    }


    // TODO: Replace this with a more natural approach, I'll use this for initial experiments of independence number, etc
    // Write something that exactly respects a parameter? (num end points or num intervals), or is the natural approach to produce an object
    // centered around some ideal?
    std::vector<cg::data_structures::Interval> generateRandomIntervalsShared(int numIntervals, int maxPerEndpoint, int maxLength, int seed)
    {
        struct InitialInterval
        {
            int Left;
            int Right;
            int Index;
        };

        std::mt19937 rng(seed);

        std::vector<InitialInterval> initialIntervals;

        initialIntervals.reserve(numIntervals);
        auto remaining = numIntervals;
        auto here = 0;
        auto intervalIndex = 0;
        
        auto minLeft = std::numeric_limits<int>::max();
        auto maxRight = std::numeric_limits<int>::min();
        while(remaining > 0)
        {
            const auto numEndpoints = std::uniform_int_distribution<>(1, std::min(maxPerEndpoint, remaining))(rng);
            for(auto i = 0; i < numEndpoints; ++i)
            {
                const auto isLeft = std::bernoulli_distribution(0.5)(rng);
                const auto length = std::uniform_int_distribution<>(1, maxLength)(rng);
                const auto interval = isLeft ? InitialInterval{ here, here + length, intervalIndex } : InitialInterval{here - length, here, intervalIndex}; 
                if(interval.Left < minLeft)
                {
                    minLeft = interval.Left;
                }
                if(interval.Right > maxRight)
                {
                    maxRight = interval.Right;
                }
                initialIntervals.push_back(interval);
                ++intervalIndex;
            }
            remaining -= numEndpoints;
            ++here;
        }

        std::vector<bool> isOccupied(1 + maxRight - minLeft, false);
        for(auto interval : initialIntervals)
        {
            auto newLeft = interval.Left - minLeft;
            auto newRight = interval.Right - minLeft;
            isOccupied[newLeft] = isOccupied[newRight] = true;
        }
        std::vector<int> cumulativeGap(isOccupied.size(), 0);
        auto numGaps = 0;
        for(auto i = 0; i < isOccupied.size(); ++i)
        {
            if(!isOccupied[i])
            {
                ++numGaps;
            }
            cumulativeGap[i] = numGaps;
        }

        std::vector<cg::data_structures::Interval> result;
        result.reserve(numIntervals);
        std::ranges::sort(initialIntervals, [](const InitialInterval &a, const InitialInterval &b)
                          { return a.Left < b.Left; });

        std::optional<InitialInterval> previousInterval;
        for(const auto& interval : initialIntervals)
        {
            if(previousInterval)
            {
                auto p = previousInterval.value();
                if(p.Left == interval.Left && p.Right == interval.Right)
                {
                    continue; 
                }
            }
            previousInterval = interval;

            auto newLeft = interval.Left - minLeft;
            auto newRight = interval.Right - minLeft;
            const auto weight = 1; // std::uniform_int_distribution<>(1, 10 * numIntervals)(rng);
            const auto newInterval = cg::data_structures::Interval(newLeft - cumulativeGap[newLeft], newRight - cumulativeGap[newRight], result.size(), weight);
            result.push_back(newInterval);
        }
        
        return result;
    }

    int getMaxRightEndpoint(std::span<const cg::data_structures::Interval> intervals)
    {
        auto it = std::max_element(intervals.begin(), intervals.end(),
        [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
            return a.Right < b.Right;  
        });

        auto maxRight = (*it).Right;
        return maxRight;
    }

    long sumWeights(std::span<cg::data_structures::Interval> intervals)
    {
        return std::accumulate(
        intervals.begin(), intervals.end(), 0,
        [](int acc, const cg::data_structures::Interval& i) { return acc + i.Weight; });
    }

    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponentsNaive(std::span<const cg::data_structures::Interval> intervals)
    {
        // Just construct the entire adjacency matrix and then DFS.
        std::vector<std::vector<bool>> isAdjacent(intervals.size(), std::vector<bool>(intervals.size()));

        for(auto i : intervals)
        {
            for(auto j : intervals)
            {
                isAdjacent[i.Index][j.Index] = i.Index != j.Index && i.overlaps(j);
            }
        }

        std::vector<bool> visited(intervals.size(), false);
        std::stack<cg::data_structures::Interval> pending;
        std::vector<std::vector<cg::data_structures::Interval>> components;
        for(auto i : intervals)
        {
            if(!visited[i.Index])
            {
                auto currentComponent = std::vector<cg::data_structures::Interval>();
                pending.push(i);
                visited[i.Index] = true;
                while(!pending.empty())
                {
                    auto j = pending.top();
                    pending.pop();
                    currentComponent.push_back(j);
                    for(auto k : intervals)
                    {
                        if(isAdjacent[j.Index][k.Index] && !visited[k.Index])
                        {
                            visited[k.Index] = true;
                            pending.push(k);
                        }
                    }
                }
                components.push_back(currentComponent);
            }
        }
        return components;
    }


    // This is the simple O(n \log^2 n) time algorithm to find the connected components of a circle graph given an interval model.
    //
    // Kim Jae-hoon, Korea Institute of Information and Communication Engineering, v.22 no.11, 2018, pp.1538 - 1543
    //
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponents(cg::data_structures::DistinctIntervalRep& intervalRep)
    {
        std::stack<std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>> componentsInProgress;

        std::vector<std::vector<cg::data_structures::Interval>> completeComponents;
        
        for(auto i = 0; i < intervalRep.end; ++i)
        {
            auto maybeInterval = intervalRep.tryGetIntervalByLeftEndpoint(i);
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                
                // Remove all components that don't intersect the current interval and mark them as complete.
                // No future interval can be connected to any of them, since it will have a larger left end point than the current interval.
                while(!componentsInProgress.empty())
                {
                    auto mostRecentComponent = componentsInProgress.top();
                    if(mostRecentComponent.empty())
                    {
                        throw std::runtime_error("Internal error: encountered an empty most recent component");
                    }
                    auto last = *mostRecentComponent.rbegin();
                    if(last.Right > interval.Left)
                    {
                        // This component may include 'interval'.
                        break;
                    }
                    // This component cannot have any more intervals, remove it from the in progress ones and add it to the complete components.
                    auto complete = std::vector<cg::data_structures::Interval>{mostRecentComponent.begin(), mostRecentComponent.end()};
                    completeComponents.push_back(complete);
                    componentsInProgress.pop();
                }
                // At this point, there are either no more components, or there are no right end-points in the top component, within interval
                if (componentsInProgress.empty())
                {
                    // Begin a new component, there is no top component
                    componentsInProgress.push(std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>{interval});        
                }
                else
                {
                    auto top = componentsInProgress.top();
                    auto it = std::prev(top.upper_bound(interval)); // Find the interval with largest right end-point before interval's right end-point.
                    if (!it->overlaps(interval))
                    {
                        componentsInProgress.push(std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>{interval});
                    }
                    else
                    {

                        // This interval intersects at least one other component.
                        auto mergedComponent = componentsInProgress.top(); // The new interval intersects this component, so it should be added here.
                        componentsInProgress.pop();
                        // However, if there are other components it may overlap them too. If that's the case, we need to merge them with 'mergedComponent'
                        // as they're all part of the same connected component of the graph.
                        while (!componentsInProgress.empty())
                        {
                            auto prevComponent = componentsInProgress.top();
                            if (!prevComponent.rbegin()->overlaps(interval))
                            {
                                break;
                            }
                            componentsInProgress.pop();
                            // Insert the smaller component into the larger, and ensure 'mergedComponent' points to the resulting component (where interval belongs)
                            // Inserting the smaller component into the larger is important so that each interval can participate in at most O(\log n) merges
                            if (mergedComponent.size() < prevComponent.size())
                            {
                                prevComponent.insert(mergedComponent.begin(), mergedComponent.end());
                                mergedComponent = prevComponent;
                            }
                            else
                            {
                                mergedComponent.insert(prevComponent.begin(), prevComponent.end());
                            }
                        }
                        mergedComponent.insert(interval);
                        componentsInProgress.push(mergedComponent);
                    }
                }
            }
        }
        while(!componentsInProgress.empty())
        {
            auto top = componentsInProgress.top();
            auto complete = std::vector<cg::data_structures::Interval>{top.begin(), top.end()};
            completeComponents.push_back(complete);
            componentsInProgress.pop();        
        }
        return completeComponents;
    }

 }
