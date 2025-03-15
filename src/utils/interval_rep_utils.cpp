#include <format>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

#include "data_structures/interval.h"
#include "data_structures/simple_interval_rep.h"

#include "utils/interval_rep_utils.h"
 
 namespace cg::utils
 {
    void verify_dense(std::span<const cg::data_structures::Interval> intervals)
    {
        auto end = 2 * intervals.size() + 1;
        std::vector<bool> alreadySeen(end, false);        
        for(auto& i : intervals)
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

    int compute_density(const cg::data_structures::SimpleIntervalRep& intervals)
    {
        auto numOpen = 0;
        auto maxOpen = 0;
        for(auto i = 0; i < intervals.End; ++i)
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
    std::vector<cg::data_structures::Interval> generate_random_intervals(int numIntervals)
    {
        std::vector<int> endPoints(2 * numIntervals);
        std::iota(endPoints.begin(), endPoints.end(), 1);
        std::mt19937 rng(2010);
        std::shuffle(endPoints.begin(), endPoints.end(), rng);

        std::vector<cg::data_structures::Interval> result;
        result.reserve(numIntervals);
        for(int i = 0; i < numIntervals; ++i)
        {
            auto leftIndex = 2 * i;
            auto leftEndpoint = endPoints[leftIndex];
            auto rightEndpoint = endPoints[leftIndex + 1];
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
