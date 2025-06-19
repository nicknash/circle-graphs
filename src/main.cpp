#include <iostream>
#include <array>
#include <ranges>
#include <format>
#include <cmath>
#include <sstream>

#include "data_structures/interval.h"
#include "data_structures/chord.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/shared_interval_model.h"
#include "data_structures/chord_model.h"
#include "data_structures/graph.h"


#include "utils/interval_model_utils.h"
#include "utils/chord_model_utils.h"
#include "utils/counters.h"
#include "utils/components.h"
#include "utils/spinrad_prime.h"

#include "mis/distinct/naive.h"
#include "mis/distinct/valiente.h"
#include "mis/distinct/switching.h"
#include "mis/distinct/pure_output_sensitive.h"
#include "mis/distinct/combined_output_sensitive.h"
#include "mis/distinct/simple_implicit_output_sensitive.h"
#include "mis/distinct/implicit_output_sensitive.h"
#include "mis/distinct/lazy_output_sensitive.h"

#include "mis/shared/naive.h"
#include "mis/shared/pure_output_sensitive.h"
#include "mis/shared/pruned_output_sensitive.h"
#include "mis/shared/valiente.h"


template<typename Range>
std::string csv_sizes(const Range& r) {
    std::ostringstream oss;
    auto it = std::begin(r);
    auto end = std::end(r);

    if (it != end) {
        // first element (no leading comma)
        oss << it->size();
        ++it;
    }
    // remaining elements, each prefixed by comma
    for (; it != end; ++it) {
        oss << ',' << it->size();
    }
    return oss.str();
}


int main()
{
/*    cg::data_structures::Graph g(12);

    g.addEdge(1, 2);
    g.addEdge(2, 6);
    g.addEdge(2, 11);
    g.addEdge(3, 11);
    g.addEdge(3, 4);
    g.addEdge(3, 10);
    g.addEdge(4, 5);
    g.addEdge(7, 11);
    g.addEdge(7, 8);
    g.addEdge(7, 9);
    g.addEdge(9, 10);
*/

/*
    cg::data_structures::Graph g(6);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 0);
*/

    cg::data_structures::Graph g(6);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 3);
    g.addEdge(0, 4);
    g.addEdge(0, 5);


    cg::utils::SpinradPrime sp;
    
    sp.isPrime(g);
    
    return 0;

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
            const auto &intervals = cg::interval_model_utils::generateRandomIntervalsShared(numIntervals, maxEndpointsPerPoint, maxLength, seed);

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


            auto sharedIntervalModel = cg::data_structures::SharedIntervalModel(intervals);
            auto totalLeft = 0;
            auto totalRight = 0;
            for(auto x = 0; x < sharedIntervalModel.end; ++x)
            {
                auto y = sharedIntervalModel.getAllIntervalsWithLeftEndpoint(x).size();
                totalLeft += y;
                //std::cout << std::format("NUM_LEFT[{}] = {}", x, y) << std::endl;
                auto z = sharedIntervalModel.getAllIntervalsWithRightEndpoint(x).size();
                //std::cout << std::format("NUM_RIGHT[{}] = {}", x, z) << std::endl;

                totalRight += z;
            }
            auto avgLeftPerEp = totalLeft / (double) sharedIntervalModel.end;
            auto avgRightPerEp = totalRight / (double) sharedIntervalModel.end;


            std::cout << std::format("Shared interval rep of {} intervals with maxEndpointsPerPoint={}, TotalEndpoints={}, TotalIntervalLength={}, MaxIntervalLength={}, avgLeftPerEp={}, avgRightPerEp={}", intervals.size(), maxEndpointsPerPoint, sharedIntervalModel.end, totalIntervalLength, maxLength, avgLeftPerEp, avgRightPerEp) << std::endl;


            auto mis1 = cg::mis::shared::Naive::computeMIS(sharedIntervalModel, naiveCounts);
            auto weight1 = cg::interval_model_utils::sumWeights(mis1);


            std::cout << std::format("\tShared naive IndependenceNumber={}, TotalWeight={}, InnerLoop={}, InnerMaxLoop={}, NormalizedInnerLoop={}, NormalizedMaxLoop={}",
                                     mis1.size(),
                                     weight1,
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerLoop) / (float)(sharedIntervalModel.end * sharedIntervalModel.end),
                                     naiveCounts.Get(cg::mis::shared::Naive::Counts::InnerMaxLoop) / (float)(sharedIntervalModel.end * numIntervals))
                      << std::endl;

            auto mis2 = cg::mis::shared::PureOutputSensitive::tryComputeMIS(sharedIntervalModel, numIntervals, osCounts).value();
            auto weight2 = cg::interval_model_utils::sumWeights(mis2);

            std::cout << std::format("\tShared output sensitive IndependenceNumber={}, TotalWeight={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", mis2.size(), weight2,
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::IntervalOuterLoop),
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop),
                                     osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop),
                                     (osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackOuterLoop) + osCounts.Get(cg::mis::shared::PureOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalModel.end * mis2.size()))
                      << std::endl;

            auto mis3 = cg::mis::shared::Valiente::computeMIS(sharedIntervalModel, valienteCounts);
            auto weight3 = cg::interval_model_utils::sumWeights(mis3);


            std::cout << std::format("\tShared valiente IndependenceNumber={}, TotalWeight={}, InnerLoop={}, InnerMaxLoop={}, NormalizedInnerLoop={}, NormalizedMaxLoop={}",
                                     mis3.size(),
                                     weight3,
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerLoop),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerMaxLoop),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerLoop) / (float)(sharedIntervalModel.end * sharedIntervalModel.end),
                                     valienteCounts.Get(cg::mis::shared::Valiente::Counts::InnerMaxLoop) / (float)(sharedIntervalModel.end * numIntervals)) << std::endl;

            auto mis4 = cg::mis::shared::PrunedOutputSensitive::tryComputeMIS(sharedIntervalModel, numIntervals, posCounts).value();
            auto weight4 = cg::interval_model_utils::sumWeights(mis4);
            auto tmp = (long)sharedIntervalModel.end * mis4.size() / (float)numIntervals;

            std::cout << std::format("\tShared pruned output sensitive PRUNEFACTOR={}, IndependenceNumber={}, TotalWeight={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", tmp, mis4.size(), weight2,
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::IntervalOuterLoop),
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackOuterLoop),
                                     posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackInnerLoop),
                                     (posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackOuterLoop) + posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalModel.end * mis2.size()))
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
    //for (int seed2 = 0; seed2 < 15000; ++seed2)
    auto seed2 = 1;
    {
    //    auto intervals = cg::interval_model_utils::generateRandomIntervals(50 + 100 * i, i);
        std::cout << "SEED = " << seed2 << std::endl;
        //auto intervals = cg::interval_model_utils::generateRandomIntervals(15000, seed2);
        
    

        //std::vector<cg::data_structures::Interval> intervals;
        /*int numLayers = 10;
        int numIntervalsAcordingToLayers = numLayers * 2;
        int intervalIdx = 0;
        for(int layer = 0; layer < numLayers; ++layer)
        {
            int e = 2 * numIntervalsAcordingToLayers - 1 - 3 * layer;
            int s = layer;
            intervals.push_back(cg::data_structures::Interval(s, e - 2, intervalIdx, 1));
            intervals.push_back(cg::data_structures::Interval(e - 1, e, intervalIdx + 1, 1));
            intervalIdx += 2;
        }*/
   
        //auto intervalModel = cg::data_structures::DistinctIntervalModel(intervals);

        auto numIntervals = 10;
        for(int x = 0; x < numIntervals; ++x)
        {
//            intervals.push_back(cg::data_structures::Interval(x, 2 * numIntervals - x - 1, x, 1)); // nested stack
  //          auto y = x + numIntervals;
    //        intervals.push_back(cg::data_structures::Interval(2 * y, 2 * y + 1, y, 1)); // disjoint units, offset (to combine with nested stack on left)


        //   intervals.push_back(cg::data_structures::Interval(2 * x, 2 * x + 1, x, 1)); // disjoint units
            //intervals.push_back(cg::data_structures::Interval(x, x + numIntervals, x, 1)); // clique
        }
        int numEndpoints = 400;
        std::vector<int> connectionSeq;
        for(int i = 1; i < numEndpoints/2; i+=13)
        {
            connectionSeq.push_back(i);
        }

        auto chordModel = cg::utils::generateChordModel(numEndpoints, connectionSeq);
        auto intervalModel = chordModel.toDistinctIntervalModel();
        auto intervals = intervalModel.getAllIntervals();
        
        
        const auto& components = cg::components::getConnectedComponents(intervals);
        std::cout << std::format("There are {} connected components of sizes: {}", components.size(), csv_sizes(components)) << std::endl;
/*
        auto mis = cg::mis::distinct::Naive::computeMIS(intervalModel);
        std::cout << std::format("Naive {}", mis.size()) << std::endl;
        for (auto i : mis)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }
*/

        auto mis2 = cg::mis::distinct::Valiente::computeMIS(intervalModel);
        std::cout << std::format("Valiente {}", mis2.size()) << std::endl;
        for (auto i : mis2)
        {
            // std::cout << std::format("{}", i) << std::endl;
        }

        cg::utils::Counters<cg::mis::distinct::PureOutputSensitive::Counts> posCounts;

        auto mis3 = cg::mis::distinct::PureOutputSensitive::tryComputeMIS(intervalModel, 1000000000, posCounts).value();
        std::cout << std::format("PureOutputSensitive {}", mis3.size()) << std::endl;
        for (auto i : mis3)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }

        //cg::utils::Counters<cg::mis::distinct::ImplicitOutputSensitive::Counts> posCounts;
        //auto mis4 = cg::mis::distinct::ImplicitOutputSensitive::tryComputeMIS(intervalModel, intervals.size(), posCounts).value();
        //std::cout << std::format("Implicit output sensitive {}", mis4.size()) << std::endl;

        cg::utils::Counters<cg::mis::distinct::SimpleImplicitOutputSensitive::Counts> siosCounts;
        auto mis5 = cg::mis::distinct::SimpleImplicitOutputSensitive::tryComputeMIS(intervalModel, intervals.size(), siosCounts).value();
        std::cout << std::format("Simple Implicit output sensitive {}", mis5.size()) << std::endl;
        
        long totalIntervalLength = 0;
        long totalChordLength = 0;

        for(auto i : intervals) 
        {
            //std::cout << std::format("{}", i) << std::endl;

            totalIntervalLength += i.length();
            totalChordLength += cg::data_structures::Chord(i.Left, i.Right, 0, 0).length(2 * intervals.size());
        }
        
        
        std::cout << std::format("NumIntervals: {}, Alpha: {}, Sqrt(NumIntervals): {}, Alpha/sqrt(NumIntervals): {}, totalLength: {}, totalLength/numIntervals^2: {}, totalChordLengthDistinctModel: {}, totalChordLengthDistinctModel/numIntervals^2: {} ", intervals.size(), mis5.size(), std::sqrt(intervals.size()), mis5.size()/std::sqrt(intervals.size()), 
        totalIntervalLength, totalIntervalLength / (float) (intervals.size() * intervals.size()), totalChordLength, totalChordLength / (float) (intervals.size() * intervals.size())) << std::endl; 


        cg::utils::Counters<cg::mis::distinct::LazyOutputSensitive::Counts> losCounts;

/*
        auto mis4 = cg::mis::distinct::LazyOutputSensitive::tryComputeMIS(intervalModel, intervals.size(), losCounts).value();

        std::cout << std::format("Lazy output sensitive {}", mis4.size()) << std::endl;
        for (auto i : mis4)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }
        cg::utils::Counters<cg::mis::distinct::CombinedOutputSensitive::Counts> cosCounts;

        auto mis6 = cg::mis::distinct::CombinedOutputSensitive::tryComputeMIS(intervalModel, intervals.size(), cosCounts).value();

        std::cout << std::format("Combined output sensitive {}", mis6.size()) << std::endl;

*/
        // std::cout << std::format("\tShared pruned output sensitive PRUNEFACTOR={}, IndependenceNumber={}, TotalWeight={}, OuterInterval={}, OuterStack={}, InnerStack={}, NormalizedStackTotal={}", tmp, mis4.size(), weight2,
        //                              posCounts.Get(Counts::IntervalOuterLoop),
        //                              posCounts.Get(Counts::StackOuterLoop),
        //                              posCounts.Get(Counts::StackInnerLoop),
        //                              (posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackOuterLoop) + posCounts.Get(cg::mis::shared::PrunedOutputSensitive::Counts::StackInnerLoop)) / (float)(sharedIntervalModel.end * mis2.size()))
        //               << std::endl;

/*        if(mis3.size() != mis4.size())
        {
            throw std::runtime_error(std::format("mis3.size() = {}, mis4.size() = {}", mis3.size(), mis4.size()));
        }*/
  /*    
        if(mis.size() != mis2.size() || mis2.size() != mis3.size() || mis3.size() != mis4.size())
        {
            throw std::runtime_error(std::format("mis.size() = {}, mis2.size() = {}, mis3.size() = {}, mis4.size() = {}", mis.size(), mis2.size(), mis3.size(), mis4.size()));
        }*/
    }
    
    return 0;
}