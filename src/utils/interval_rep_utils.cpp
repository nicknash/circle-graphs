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
        throw std::exception();
    }

    std::vector<cg::data_structures::Interval> generate_random_intervals(int numIntervals)
    {
        std::vector<cg::data_structures::Interval> result(2 * numIntervals);
        std::iota(result.begin(), result.end(), 1);
        std::mt19937 rng(2010);
        std::shuffle(result.begin(), result.end(), rng);
    }
 }
