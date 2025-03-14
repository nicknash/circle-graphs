#include <format>
#include <stdexcept>
#include <vector>

#include "data_structures/interval.h"

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
 }
