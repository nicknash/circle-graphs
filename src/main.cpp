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
#include "mis/distinct/implicit_output_sensitive.h"

#include "mis/shared/naive.h"
#include "mis/shared/pure_output_sensitive.h"
#include "mis/shared/pruned_output_sensitive.h"
#include "mis/shared/valiente.h"

#include <array>


int main()
{
//5 4 4 4 3 3 3 3 2 2 2 2 1 1 1 1 1 1 1 1 0 0 0 0 0
//5 4 4 4 3 3 3 3 2 2 2 2 1 1 1 1 1 1 1 1 0 0 0 0 0 0
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

//
/*
    cg::mis::distinct::MonotoneSeq seq(25);

    for(int i = 0; i < 5; ++i)
    {
        seq.increment(0);
    }

    for(int i = 0; i < 4; ++i)
    {
        seq.increment(3);
    }

    for(int i = 0; i < 3; ++i)
    {
        seq.increment(7);
    }

    for(int i = 0; i < 2; ++i)
    {
        seq.increment(11);
    }

    seq.increment(19);

    std::vector<int> blah(25, 0);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout <<  std::endl;

    seq.increment(12);

    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout <<  std::endl;

    return 0;
  */
  
  /*
    // Get a unit test framework, this is a disaster...
  
    cg::mis::distinct::MonotoneSeq seq(10);
    std::vector<int> blah(10, 0);

    auto x = seq.increment(5);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }

    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;

    x = seq.increment(0);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;

    x = seq.increment(0);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }

    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;


    x = seq.increment(0);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;


    x = seq.increment(3);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;



    x = seq.increment(4);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;

    x = seq.increment(4);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;

    x = seq.increment(5);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;

    x = seq.increment(9);
    seq.copyTo(blah);

    for(int i = 0; i < blah.size(); ++i)
    {
        std::cout << blah[i] << " ";
    }
    std::cout << std::format("[{},{}]", x.left, x.right) <<  std::endl;


    return 0;
*/

    /*cg::data_structures::NonDecreasingSeq seq(10);

    for(int i = 0; i < seq.size; ++i)
    {
        seq.increase(seq.size - i - 1);
        for(int j = 0; j < seq.size; ++j)
        {
            std::cout << seq.get(j) << " ";
        }
        std::cout << std::endl;
    }
    return 0;
*/
    // for (int seed = 0; seed < 10000; ++seed)
    auto seed = 89;
    //const auto numIntervals = 5000;
/*
    const auto numIntervals = 100000;
    int maxEndpointsPerPoint = 1200;
    //for (int maxEndpointsPerPoint = 1; maxEndpointsPerPoint < 64; maxEndpointsPerPoint *= 2)
    {
        std::cout << std::format(" **** maxEndpointsPerPoint = {} **** ", maxEndpointsPerPoint) << std::endl;
        //for (int maxLength = 4; maxLength < 2 * numIntervals; maxLength *= 2)
        auto maxLength = 200;//numIntervals/10;
        {

            // std::cout << seed << std::endl;
            const auto &intervals = cg::utils::generateRandomIntervalsShared(numIntervals, maxEndpointsPerPoint, maxLength, seed);

            auto totalIntervalLength = 0;
            
            for (const auto &i : intervals)
            {
                totalIntervalLength += i.length();
                
                //std::cout << std::format("{}", i) << std::endl;
            }

            cg::utils::Counters<cg::mis::shared::PureOutputSensitive::Counts> osCounts;
            cg::utils::Counters<cg::mis::shared::PrunedOutputSensitive::Counts> posCounts;
            cg::utils::Counters<cg::mis::shared::Naive::Counts> naiveCounts;
            cg::utils::Counters<cg::mis::shared::Valiente::Counts> valienteCounts;


            auto sharedIntervalRep = cg::data_structures::SharedIntervalRep(intervals);
            auto totalLeft = 0;
            auto totalRight = 0;
            for(auto x = 0; x < sharedIntervalRep.end; ++x)
            {
                auto y = sharedIntervalRep.getAllIntervalsWithLeftEndpoint(x).size();
                totalLeft += y;
                //std::cout << std::format("NUM_LEFT[{}] = {}", x, y) << std::endl;
                auto z = sharedIntervalRep.getAllIntervalsWithRightEndpoint(x).size();
                //std::cout << std::format("NUM_RIGHT[{}] = {}", x, z) << std::endl;

                totalRight += z;
            }
            auto avgLeftPerEp = totalLeft / (double) sharedIntervalRep.end;
            auto avgRightPerEp = totalRight / (double) sharedIntervalRep.end;


            std::cout << std::format("Shared interval rep of {} intervals with maxEndpointsPerPoint={}, TotalEndpoints={}, TotalIntervalLength={}, MaxIntervalLength={}, avgLeftPerEp={}, avgRightPerEp={}", intervals.size(), maxEndpointsPerPoint, sharedIntervalRep.end, totalIntervalLength, maxLength, avgLeftPerEp, avgRightPerEp) << std::endl;


            auto mis1 = cg::mis::shared::Naive::computeMIS(sharedIntervalRep, naiveCounts);
            auto weight1 = cg::utils::sumWeights(mis1);


            std::cout << std::format("\tShared naive IndependenceNumber={}, TotalWeight={}, InnerLoop={}, InnerMaxLoop={}, NormalizedInnerLoop={}, NormalizedMaxLoop={}",
                                     mis1.size(),
                                     weight1,
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop) / (float)(sharedIntervalRep.end * sharedIntervalRep.end),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop) / (float)(sharedIntervalRep.end * numIntervals))
                      << std::endl;

            auto mis2 = cg::mis::shared::PureOutputSensitive::tryComputeMIS(sharedIntervalRep, numIntervals, osCounts).value();
            auto weight2 = cg::utils::sumWeights(mis2);

            std::cout << std::format("\tShared output sensitive IndependenceNumber={}, TotalWeight={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", mis2.size(), weight2,
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::IntervalOuterLoop),
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop),
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop),
                                     (osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop) + osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalRep.end * mis2.size()))
                      << std::endl;

            auto mis3 = cg::mis::shared::Valiente::computeMIS(sharedIntervalRep, valienteCounts);
            auto weight3 = cg::utils::sumWeights(mis3);


            std::cout << std::format("\tShared valiente IndependenceNumber={}, TotalWeight={}, InnerLoop={}, InnerMaxLoop={}, NormalizedInnerLoop={}, NormalizedMaxLoop={}",
                                     mis3.size(),
                                     weight3,
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerLoop),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerMaxLoop),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerLoop) / (float)(sharedIntervalRep.end * sharedIntervalRep.end),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerMaxLoop) / (float)(sharedIntervalRep.end * numIntervals)) << std::endl;

            auto mis4 = cg::mis::shared::PrunedOutputSensitive::tryComputeMIS(sharedIntervalRep, numIntervals, posCounts).value();
            auto weight4 = cg::utils::sumWeights(mis4);
            auto tmp = (long)sharedIntervalRep.end * mis4.size() / (float)numIntervals;

            std::cout << std::format("\tShared pruned output sensitive PRUNEFACTOR={}, IndependenceNumber={}, TotalWeight={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", tmp, mis4.size(), weight2,
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::IntervalOuterLoop),
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackOuterLoop),
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackInnerLoop),
                                     (posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackOuterLoop) + posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalRep.end * mis2.size()))
                      << std::endl;

            if (mis1.size() != mis2.size() || mis2.size() != mis3.size() || mis3.size() != mis4.size())
            {
                throw std::runtime_error(std::format("mis1.size() = {}, mis2.size() = {}, mis3.size() = {}, mis4.size() = {}", mis1.size(), mis2.size(), mis3.size(), mis4.size()));
            }

            if (weight1 != weight2)
            {
                throw std::runtime_error(std::format("weight1 = {}, weight2", weight1, weight2));
            }
            // if (maxLength < 20)
            // {
            //     ++maxLength;
            // }
            // else
            // {
            //     maxLength *= 2;
            // }
        }
    }*/

    
    //for (int i = 0; i < 50; ++i)
    //for (int seed2 = 0; seed2 < 150; ++seed2)
    auto seed2 = 6;
    {
    //    auto intervals = cg::utils::generateRandomIntervals(50 + 100 * i, i);
        std::cout << "SEED = " << seed2 << std::endl;
        auto intervals = cg::utils::generateRandomIntervals(25, seed2);
        auto intervalRep = cg::data_structures::DistinctIntervalRep(intervals);

        for (auto i : intervals)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }
/*
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

        auto mis3 = cg::mis::distinct::PureOutputSensitive::tryComputeMIS(intervalRep, intervals.size()).value();
        std::cout << std::format("PureOutputSensitive {}", mis3.size()) << std::endl;
        for (auto i : mis3)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }*/
        auto mis4 = cg::mis::distinct::ImplicitOutputSensitive::tryComputeMIS(intervalRep, intervals.size()).value();

        std::cout << std::format("Implicit output sensitive {}", mis4.size()) << std::endl;
        for (auto i : mis4)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }
  /*    
        if(mis.size() != mis2.size() || mis2.size() != mis3.size() || mis3.size() != mis4.size())
        {
            throw std::runtime_error(std::format("mis.size() = {}, mis2.size() = {}, mis3.size() = {}, mis4.size() = {}", mis.size(), mis2.size(), mis3.size(), mis4.size()));
        }*/
    }
    
    return 0;
}