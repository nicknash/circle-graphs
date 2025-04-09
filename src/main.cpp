#include <iostream>

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_rep.h"
#include "data_structures/shared_interval_rep.h"
#include "utils/interval_rep_utils.h"
#include "utils/counters.h"

#include "mis/distinct/naive.h"
#include "mis/distinct/valiente.h"
#include "mis/distinct/switching.h"
#include "mis/distinct/pure_output_sensitive.h"

#include "mis/shared/naive.h"
#include "mis/shared/pure_output_sensitive.h"

#include <array>



int main()
{
    

    //for (int seed = 0; seed < 10000; ++seed)
    auto seed = 89;
        const auto numIntervals = 5000;

        for (int maxEndpointsPerPoint = 1; maxEndpointsPerPoint < 32; maxEndpointsPerPoint *= 2)
        {
            std::cout << std::format(" **** maxEndpointsPerPoint = {} **** ", maxEndpointsPerPoint) << std::endl;
            for (int maxLength = 4; maxLength < 2 * numIntervals;)
            {


                // std::cout << seed << std::endl;
                const auto &intervals = cg::utils::generateRandomIntervalsShared(numIntervals, maxEndpointsPerPoint, maxLength, seed);

                auto totalIntervalLength = 0;
                for (const auto &i : intervals)
                {
                    totalIntervalLength += i.length();
                }

                cg::utils::Counters<cg::mis::shared::PureOutputSensitive::Counts> osCounts;
                cg::utils::Counters<cg::mis::shared::Naive::Counts> naiveCounts;

                auto sharedIntervalRep = cg::data_structures::SharedIntervalRep(intervals);
                auto mis1 = cg::mis::shared::Naive::computeMIS(sharedIntervalRep, naiveCounts);

                std::cout << std::format("Shared interval rep of {} intervals with maxEndpointsPerPoint={}, TotalEndpoints={}, TotalIntervalLength={}, MaxIntervalLength={}", numIntervals, maxEndpointsPerPoint, sharedIntervalRep.end, totalIntervalLength, maxLength) << std::endl;

                std::cout << std::format("\tShared naive IndependenceNumber={}, InnerLoop={}, InnerMaxLoop={}, NormalizedInnerLoop={}, NormalizedMaxLoop={}", 
                mis1.size(), 
                naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop), 
                naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop), 
                naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop) / (float) (sharedIntervalRep.end * sharedIntervalRep.end), 
                naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop) / (float) (sharedIntervalRep.end * numIntervals)) << std::endl;

                auto mis2 = cg::mis::shared::PureOutputSensitive::tryComputeMIS(sharedIntervalRep, numIntervals, osCounts).value();

                std::cout << std::format("\tShared output sensitive IndependenceNumber={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", mis2.size(),
                                         osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::IntervalOuterLoop),
                                         osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop),
                                         osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop),
                                         (osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop) + osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalRep.end * mis2.size()))
                          << std::endl;

                if (mis1.size() != mis2.size())
                {
                    throw std::runtime_error(std::format("mis1.size() = {}, mis2.size() = {}", mis1.size(), mis2.size()));
                }

                if (maxLength < 20)
                {
                    ++maxLength;
                }
                else
                {
                    maxLength *= 2;
                }
            }
        }

    /*
    for (int i = 0; i < 50; ++i)
    {
        auto intervals = cg::utils::generateRandomIntervals(50 + 100 * i, i);

        auto intervalRep = cg::data_structures::DistinctIntervalRep(intervals);

        auto mis = cg::mis::distinct::Naive::computeMIS(intervalRep);
        std::cout << std::format("Naive {}", mis.size()) << std::endl;
        for (auto i : mis)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }

        auto mis2 = cg::mis::distinct::Valiente::computeMIS(intervalRep);
        std::cout << std::format("Valiente {}", mis2.size()) << std::endl;
        for (auto i : mis2)
        {
            // std::cout << std::format("{}", i) << std::endl;
        }

        auto mis3 = cg::mis::distinct::Switching::computeMIS(intervalRep);
        std::cout << std::format("Switching {}", mis3.size()) << std::endl;
        for (auto i : mis3)
        {
            // std::cout << std::format("{}", i) << std::endl;
        }
    
        auto sharedIntervalRep = cg::data_structures::SharedIntervalRep(intervals);
        auto mis4 = cg::mis::shared::Naive::computeMIS(sharedIntervalRep);

        std::cout << std::format("Shared naive {}", mis4.size()) << std::endl;

        auto mis5 = cg::mis::shared::PureOutputSensitive::tryComputeMIS(sharedIntervalRep, 0).value();

        std::cout << std::format("Shared output sensitive {}", mis5.size()) << std::endl;

        if(mis.size() != mis2.size() || mis2.size() != mis3.size() || mis3.size() != mis4.size() || mis4.size() != mis5.size())
        {
            throw std::runtime_error(std::format("mis.size() = {}, mis2.size() = {}, mis3.size() = {}, mis4.size() = {}, mis5.size() = {}", mis.size(), mis2.size(), mis3.size(), mis4.size(), mis5.size()));
        }
    }
    */
    return 0;
}