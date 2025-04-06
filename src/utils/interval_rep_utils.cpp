#include <format>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>
#include <ranges>
#include <random>

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_rep.h"

#include "utils/interval_rep_utils.h"
 
 namespace cg::utils
 {
    void verifyEndpointsDense(std::span<const cg::data_structures::Interval> intervals)
    {
        auto end = 2 * intervals.size();
        std::vector<bool> alreadySeen(end, false);        
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
                throw std::runtime_error(std::format("Equal end-points encountered {} and {}", a.point, b.point));
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
            result.emplace_back(leftEndpoint, rightEndpoint, i);
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
 }
