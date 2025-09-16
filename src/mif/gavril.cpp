#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

#include <algorithm>
#include <stack>

namespace cg::mif
{
    void Gavril::computeRightForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<int>& rightForestSizes, array3<int>& dummyRightForestSizes)
    {
        // Collect all end-points, in increasing order.
        std::vector<int> firstLayerEndpoints;
        for(const auto& interval : firstLayerIntervals)
        {
            firstLayerEndpoints.push_back(interval.Left);
            firstLayerEndpoints.push_back(interval.Right);
        }
        std::sort(firstLayerEndpoints.begin(), firstLayerEndpoints.end());
        // Collect all intervals, ordered by decreasing right-endpoint.
        std::vector<cg::data_structures::Interval> firstLayerIntervalsByDecreasingRight(firstLayerIntervals);
        std::sort(firstLayerIntervalsByDecreasingRight.begin(), firstLayerIntervalsByDecreasingRight.end(),
          [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
              return b.Right < a.Right;
          });  

        // The base case for rightForestSizes and dummyRightForestSizes:
        // We iterate in decreasing order of right end-point 
        for(auto interval : firstLayerIntervalsByDecreasingRight) // 'interval' is 'w' in Gavril's notation
        {
            for(auto y : firstLayerEndpoints) // All end-points y such that: interval.Right < y <= last endpoint at layer 0  
            {
                if(y <= interval.Right)
                {
                    continue;
                }
                int maxDummyForestSize = 0;
                for (const auto &potentialDummyChild : firstLayerIntervalsByDecreasingRight) // Now find the dummy child with largest FR_{v, i}[l_v, y]
                {
                    auto isWithinRange = potentialDummyChild.Right <= y; 
                    auto isDummyChild = potentialDummyChild.Left > interval.Right;
                    if (isWithinRange && isDummyChild) // Check that the dummy is within (r_w, y], i.e. within (interval.Right, y]
                    {
                        const auto& dummyChild = potentialDummyChild;
                        const auto dummyForestSizeHere = rightForestSizes(dummyChild.Left + 1, y, dummyChild.Index, 0);
                        if(dummyForestSizeHere > maxDummyForestSize)
                        {
                            maxDummyForestSize = dummyForestSizeHere;
                        }
                    }
                }
                dummyRightForestSizes(y, interval.Index, 0) = maxDummyForestSize;
            }
            for(auto x : firstLayerEndpoints) // All end-points x such that: interval.Left < x <= interval.Right
            {
                if(x <= interval.Left)
                {
                    continue;
                }
                if(x > interval.Right)
                {
                    break;
                }
                for(auto y : firstLayerEndpoints) // All end-points y such that: interval.Right <= y <= last endpoint at layer 0
                {
                    if(y < interval.Right)
                    {
                        continue;
                    }
                    int maxForestSizeFromRealChild = 0;
                    for (const auto &potentialRightChild : firstLayerIntervalsByDecreasingRight)
                    {
                        auto isWithinRange = x <= potentialRightChild.Left && potentialRightChild.Right <= y;
                        auto isRealChild = potentialRightChild.Left < interval.Right && interval.Right < potentialRightChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &rightChild = potentialRightChild; // 'rightChild' is 'v' in Gavril's notation
                            auto xPrime = rightChild.Left + 1;
                            const auto forestSizeHere = rightForestSizes(xPrime, y, rightChild.Index, 0);
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                            }
                        }
                    }
                    const auto dummySize = dummyRightForestSizes(y, interval.Index, 0); // We only fill dummyRightForestSizes when y > interval.Right, so if y == interval.Right we get the pre-filled zero here, which is fine.
                    rightForestSizes(x, y, interval.Index, 0) = 1 + std::max(maxForestSizeFromRealChild, dummySize);
                }
            }
        }
    }

    void Gavril::computeLeftForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<int>& leftForestSizes)
    {
        // Collect all end-points, in increasing order.
        std::vector<int> firstLayerEndpoints;
        for(const auto& interval : firstLayerIntervals)
        {
            firstLayerEndpoints.push_back(interval.Left);
            firstLayerEndpoints.push_back(interval.Right);
        }
        std::sort(firstLayerEndpoints.begin(), firstLayerEndpoints.end());
        // Collect all intervals, ordered by increasing right-endpoint.
        std::vector<cg::data_structures::Interval> firstLayerIntervalsByIncreasingLeft(firstLayerIntervals);
        std::sort(firstLayerIntervalsByIncreasingLeft.begin(), firstLayerIntervalsByIncreasingLeft.end(),
          [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
              return a.Left < b.Left;
          });  
        // The base case for leftForestSizes: 
        // At the base case layer, 0, there are no dummy children to consider. A left dummy child is contained in the parent interval, but 
        // by definition layer 0 is the intervals containing no other interval.
        // We iterate in increasing order of left end-point
        for(auto interval : firstLayerIntervalsByIncreasingLeft) // 'interval' is 'w' in Gavril's notation
        {
            for(auto z : firstLayerEndpoints) // All end-points z such that: first endpoint at layer 0 <= z <= interval.Left
            {
                if(z > interval.Left)
                {
                    break;
                }
                for(auto q : firstLayerEndpoints) // All end-points q such that: interval.Left <= q < interval.Right
                {
                    if(q < interval.Left)
                    {
                        continue;
                    }
                    if(q >= interval.Right)
                    {
                        break;
                    }
                    int maxForestSizeFromRealChild = 0;
                    for (const auto &potentialLeftChild : firstLayerIntervalsByIncreasingLeft)
                    {
                        auto isWithinRange = z <= potentialLeftChild.Left && potentialLeftChild.Right <= q;
                        auto isRealChild = potentialLeftChild.Left < interval.Left && interval.Left < potentialLeftChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &leftChild = potentialLeftChild; // 'leftChild' is 'v' in Gavril's notation
                            auto qPrime = leftChild.Right - 1;
                            const auto forestSizeHere = leftForestSizes(z, qPrime, leftChild.Index, 0);
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                            }
                        }
                    }
                    leftForestSizes(z, q, interval.Index, 0) = 1 + maxForestSizeFromRealChild;
                }
            }
        }
    }

    void Gavril::computeNewIntervalRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer, const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer, 
        Gavril::Forests& forests)
    {
        // Collect all end-points, in increasing order.
        std::vector<int> allEndpoints; // EP_i in Gavril's notation (with layerIdx the same as Gavril's i - 1 here.)
        for(const auto& interval : allIntervalsBeforeThisLayer)
        {
            allEndpoints.push_back(interval.Left);
            allEndpoints.push_back(interval.Right);
        }
        std::sort(allEndpoints.begin(), allEndpoints.end());
        const auto previousLayerIdx = layerIdx - 1;
        for(const auto& newInterval : newIntervalsAtThisLayer) // This is w \in V_i - V_{i-1} in Gavril's notation
        {
            // First fill in FR_{w, i-1}(r_w, y] for i = layerIdx
            for(auto y : allEndpoints)
            {
                if(y <= newInterval.Right)
                {
                    continue;
                }
                int maxDummyForestSize = 0;
                for (const auto &potentialDummyChild : allIntervalsBeforeThisLayer) // Now find the dummy child with largest FR_{v, i-1}[l_v, y]
                {
                    auto isWithinRange = potentialDummyChild.Right <= y; 
                    auto isDummyChild = potentialDummyChild.Left > newInterval.Right;
                    if (isWithinRange && isDummyChild) // Check that the dummy is within (r_w, y], i.e. within (interval.Right, y]
                    {
                        const auto& dummyChild = potentialDummyChild;
                        const auto dummyForestSizeHere = forests.rightForestSizes(dummyChild.Left + 1, y, dummyChild.Index, previousLayerIdx);
                        if(dummyForestSizeHere > maxDummyForestSize)
                        {
                            maxDummyForestSize = dummyForestSizeHere;
                        }
                    }
                }
                forests.dummyRightForestSizes(y, newInterval.Index, previousLayerIdx) = maxDummyForestSize;
            }

            for(auto x : allEndpoints)
            {
                if(x <= newInterval.Left)
                {
                    continue;
                }
                if(x > newInterval.Right)
                {
                    break;
                }
                for(auto y : allEndpoints)
                {
                    if(y < newInterval.Right)
                    {
                        continue;
                    }
                    auto maxRealChildForestSize = 0;
                    for(const auto& earlierInterval : allIntervalsBeforeThisLayer)
                    {
                        auto isWithinRange = x <= earlierInterval.Left && earlierInterval.Right <= y;
                        auto isRightChild = earlierInterval.Left < newInterval.Right && newInterval.Right < earlierInterval.Right;
                        if(!isWithinRange || !isRightChild)
                        {
                            continue;
                        }
                        // Now try all end-points [q', x'] that enclose the right end-point of newInterval
                        for(auto qPrime : allEndpoints)
                        {
                            if(qPrime < earlierInterval.Left)
                            {
                                continue;
                            }
                            if(qPrime >= newInterval.Right)
                            {
                                break;
                            }
                            for(auto xPrime : allEndpoints)
                            {
                                if(xPrime <= newInterval.Right)
                                {
                                    continue;
                                }
                                if(xPrime > earlierInterval.Right)
                                {
                                    break;
                                }
                                auto rightForestSizeHere = forests.rightForestSizes(xPrime, y, earlierInterval.Index, previousLayerIdx);
                                int sizeHere = 0; 
                                if(previousLayerIdx > 0)
                                {
                                    auto leftForestSizeHere = forests.leftForestSizes(x, qPrime, earlierInterval.Index, previousLayerIdx - 1);
                                    // Note leftForestSizeHere and rightForestSizeHere both count 'earlierInterval'
                                    // (i.e, FR_{i,v}[x, y] includes v and FL_{i, v}[x, y] includes v) so the '- 1'
                                    // avoids counting 'earlierInterval'  twice.

                                    // TODONICK: look up innerSize(qPrime, xPrime, previousLayerIdx - 1)
                                    auto innerSize = 0;
                                    sizeHere = leftForestSizeHere + rightForestSizeHere + innerSize - 1;
                                }
                                else
                                {
                                    sizeHere = rightForestSizeHere; 
                                }
                                if(sizeHere > maxRealChildForestSize)
                                {
                                    maxRealChildForestSize = sizeHere;
                                }
                            }
                        }
                    }
                    auto maxDummyForestSize = forests.dummyRightForestSizes(y, newInterval.Index, previousLayerIdx);
                    forests.rightForestSizes(x, y, newInterval.Index, previousLayerIdx) = 1 + std::max(maxRealChildForestSize, maxDummyForestSize);
                }
            }
        }
    }

    void Gavril::computeRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& allIntervals, Forests2& forests, array4<ChildChoice>& rightChildChoices)
    {
        // Collect all end-points, in increasing order.
        std::vector<int> endpoints;
        for(const auto& interval : allIntervals)
        {
            endpoints.push_back(interval.Left);
            endpoints.push_back(interval.Right);
        }
        std::sort(endpoints.begin(), endpoints.end());
        // Collect all intervals, ordered by decreasing right-endpoint.
        std::vector<cg::data_structures::Interval> intervalsByDecreasingRight(allIntervals);
        std::sort(intervalsByDecreasingRight.begin(), intervalsByDecreasingRight.end(),
          [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
              return b.Right < a.Right;
          });  

        // We iterate in decreasing order of right end-point 
        for(auto interval : intervalsByDecreasingRight) // 'interval' is 'w' in Gavril's notation
        {
            for(auto y : endpoints) // All end-points y such that: interval.Right < y <= last endpoint at layer 0  
            {
                if(y <= interval.Right)
                {
                    continue;
                }
                int maxDummyForestSize = 0;
                int bestDummyIntervalIdx = -1;
                for (const auto &potentialDummyChild : intervalsByDecreasingRight) // Now find the dummy child with largest FR_{v, i}[l_v, y]
                {
                    auto isWithinRange = potentialDummyChild.Right <= y; 
                    auto isDummyChild = potentialDummyChild.Left > interval.Right;
                    if (isWithinRange && isDummyChild) // Check that the dummy is within (r_w, y], i.e. within (interval.Right, y]
                    {
                        const auto& dummyChild = potentialDummyChild;
                        const auto dummyForestSizeHere = forests.rightForestScores(dummyChild.Left + 1, y, dummyChild.Index, layerIdx).score;
                        if(dummyForestSizeHere > maxDummyForestSize)
                        {
                            maxDummyForestSize = dummyForestSizeHere;
                            bestDummyIntervalIdx = potentialDummyChild.Index;
                        }
                    }
                }
                DummyForestScore dummyScore {
                    .score = maxDummyForestSize,
                    .childIntervalIdx = bestDummyIntervalIdx
                };
                forests.dummyRightForestChoices(y, interval.Index, layerIdx) = dummyScore;
            }
            for(auto x : endpoints) // All end-points x such that: interval.Left < x <= interval.Right
            {
                if(x <= interval.Left)
                {
                    continue;
                }
                if(x > interval.Right)
                {
                    break;
                }
                for(auto y : endpoints) // All end-points y such that: interval.Right <= y <= last endpoint at layer 0
                {
                    if(y < interval.Right)
                    {
                        continue;
                    }
                    int maxRealChildForestSize = 0;
                    int bestChildIntervalIdx = -1;
                    auto dummyScore = forests.dummyRightForestChoices(y, interval.Index, layerIdx);
                    for (const auto &potentialRightChild : intervalsByDecreasingRight)
                    {
                        auto isWithinRange = x <= potentialRightChild.Left && potentialRightChild.Right <= y;
                        auto isRealChild = potentialRightChild.Left < interval.Right && interval.Right < potentialRightChild.Right;
                        if(!isWithinRange || !isRealChild)
                        {
                            continue;
                        }
                        const auto &rightChild = potentialRightChild; // 'rightChild' is 'v' in Gavril's notation

                        for(auto qPrime : endpoints)
                        {
                            if(qPrime < rightChild.Left)
                            {
                                continue;
                            }
                            if(qPrime >= interval.Right)
                            {
                                break;
                            }
                            for(auto xPrime : endpoints)
                            {
                                if(xPrime <= interval.Right)
                                {
                                    continue;
                                }
                                if(xPrime > rightChild.Right)
                                {
                                    break;
                                }
                                auto innerSize = rightChildChoices(qPrime, xPrime, interval.Index, layerIdx - 1).score;
                                auto leftForestSize = forests.leftForestChoices(x, qPrime, rightChild.Index, layerIdx - 1).score;
                                auto rightForestSize = forests.rightForestScores(xPrime, y, rightChild.Index, layerIdx).score; 
                                int sizeHere = innerSize + leftForestSize + rightForestSize - 1;
                                if(sizeHere > maxRealChildForestSize)
                                {
                                    maxRealChildForestSize = sizeHere;
                                    bestChildIntervalIdx = rightChild.Index;
                                }
                            }
                        }
                    }
                    ForestScore score;
                    if(dummyScore.score > maxRealChildForestSize)
                    {
                        score = ForestScore
                        {
                            .childType = ChildType::Dummy,
                            .score = 1 + dummyScore.score,
                            .childIntervalIdx = dummyScore.childIntervalIdx
                        };
                    }
                    else
                    {
                        score = ForestScore  
                        {
                            .childType = ChildType::Real,
                            .score = 1 + maxRealChildForestSize,
                            .childIntervalIdx = bestChildIntervalIdx
                        };
                    }
                    forests.rightForestScores(x, y, interval.Index, layerIdx) = score;
                }
            }
        }
    }

    void Gavril::computeRightChildChoices(const Forests2 &forests, const std::vector<cg::data_structures::Interval> allIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervalsOneBehind, array4<ChildChoice> &rightChildChoices, int layerIdx)
    {
        std::vector<int> endpoints;
        for (const auto &interval : cumulativeIntervals)
        {
            endpoints.push_back(interval.Left);
            endpoints.push_back(interval.Right);
        }
        std::sort(endpoints.begin(), endpoints.end());

        std::vector<int> endpointsOneBehind;
        for (const auto &interval : cumulativeIntervalsOneBehind)
        {
            endpointsOneBehind.push_back(interval.Left);
            endpointsOneBehind.push_back(interval.Right);
        }
        std::sort(endpointsOneBehind.begin(), endpointsOneBehind.end());

        for (const auto &interval : allIntervals)
        {
            for (auto x : endpoints)
            {
                for (auto y : endpoints)
                {
                    int bestChildScore = 0;
                    int bestChildIntervalIndex = -1;
                    int bestZPrime = -1;
                    int bestQPrime = -1;
                    auto rightScore = forests.rightForestScores(x, y, interval.Index, layerIdx);
                    auto childType = rightScore.childType;
                    if (childType == ChildType::Dummy)
                    {
                        bestChildScore = rightScore.score;
                        bestChildIntervalIndex = rightScore.childIntervalIdx;
                    }
                    else
                    {
                        for (const auto &child : cumulativeIntervals)
                        {
                            for (auto zPrime : endpointsOneBehind)
                            {
                                for (auto qPrime : endpointsOneBehind)
                                {
                                    auto isValidInner = interval.Left < x &&
                                                        x <= child.Left &&
                                                        child.Left <= zPrime &&
                                                        zPrime < interval.Right &&
                                                        qPrime <= child.Right &&
                                                        child.Right < y;
                                    if (!isValidInner)
                                    {
                                        continue;
                                    }
                                    int scoreHere = forests.leftForestChoices(x, zPrime, child.Index, layerIdx).score +
                                                    forests.rightForestScores(qPrime, y, child.Index, layerIdx).score +
                                                    rightChildChoices(zPrime, qPrime, interval.Index, layerIdx - 1).score - 1;
                                    if (scoreHere > bestChildScore)
                                    {
                                        bestChildScore = scoreHere;
                                        bestChildIntervalIndex = child.Index;
                                        bestQPrime = qPrime;
                                        bestZPrime = zPrime;
                                    }
                                }
                            }
                        }
                    }
                    ChildChoice childChoice
                    {
                        .childType = childType,
                        .score = bestChildScore,
                        .qPrime = bestZPrime,
                        .xPrime = bestQPrime,
                        .childIntervalIdx = bestChildIntervalIndex,
                    };
                    rightChildChoices(x, y, interval.Index, layerIdx) = childChoice;
                }
            }
        }
    }

    void Gavril::constructMif(const cg::data_structures::DistinctIntervalModel intervalModel, int numLayers, const Forests2& forests, const ChildChoices& childChoices)
    {
        const auto &allIntervals = intervalModel.getAllIntervals();

        std::vector<int> endpoints;
        for (const auto &interval : allIntervals)
        {
            endpoints.push_back(interval.Left);
            endpoints.push_back(interval.Right);
        }
        std::sort(endpoints.begin(), endpoints.end());

        std::vector<int> mifIntervalIdxs;

        int topLayer = numLayers - 1; 
        int bestScore = 0;
        int bestSplit = -1;
        int bestIntervalIndex = -1;
        int z = 0;
        int y = intervalModel.end - 1;
        for (const auto &w : allIntervals)
        {
            for (auto split = w.Left; split < w.Right; ++split)
            {
                auto scoreHere = forests.leftForestChoices(z, split, w.Index, topLayer).score +
                                 forests.rightForestScores(split + 1, y, w.Index, topLayer).score - 1;
                if (scoreHere > bestScore)
                {
                    bestScore = scoreHere;
                    bestSplit = split;
                    bestIntervalIndex = w.Index;
                }
            }
        }
        mifIntervalIdxs.push_back(bestIntervalIndex);
        struct ForestToBuild
        {
            bool isLeft;
            int rootIdx;
            int layerIdx;
            int start;
            int end;
        };
        std::stack<ForestToBuild> pending;
        pending.push(ForestToBuild
            {
                .isLeft = true,
                .rootIdx = bestIntervalIndex,
                .layerIdx = topLayer,
                .start = z,
                .end = bestSplit
            });
        pending.push(ForestToBuild
            {
                .isLeft = false,
                .rootIdx = bestIntervalIndex,
                .layerIdx = topLayer,
                .start = bestSplit + 1,
                .end = y
            });
        while(!pending.empty())
        {
            auto f = pending.top();
            pending.pop();
            if(f.isLeft)
            {
                auto childChoice = childChoices.leftChildChoices(f.start, f.end, f.rootIdx, f.layerIdx);
                auto start = f.start;
                auto end = f.end;
                auto layerIdx = f.layerIdx;
                while (childChoice.childType == ChildType::Real)
                {
                    mifIntervalIdxs.push_back(childChoice.childIntervalIdx);
                    pending.push(ForestToBuild{
                        .isLeft = true,
                        .rootIdx = childChoice.childIntervalIdx,
                        .layerIdx = layerIdx,
                        .start = start,
                        .end = childChoice.qPrime});
                    if (layerIdx > 0)
                    {
                        pending.push(ForestToBuild{
                            .isLeft = false,
                            .rootIdx = childChoice.childIntervalIdx,
                            .layerIdx = layerIdx - 1,
                            .start = childChoice.xPrime,
                            .end = end});
                    }
                    start = childChoice.qPrime;
                    end = childChoice.xPrime;
                    --layerIdx;
                    childChoice = childChoices.leftChildChoices(start, end, f.rootIdx, layerIdx);
                }
                if(childChoice.childType == ChildType::Dummy)
                {
                    mifIntervalIdxs.push_back(childChoice.childIntervalIdx);
                    const auto& dummyRoot = intervalModel.getIntervalByIndex(childChoice.childIntervalIdx);    
                    pending.push(ForestToBuild{
                        .isLeft = true,
                        .rootIdx = childChoice.childIntervalIdx,
                        .layerIdx = layerIdx - 1,
                        .start = start,
                        .end = dummyRoot.Right - 1
                    });
                } 
            }
            else
            {
                auto childChoice = childChoices.rightChildChoices(f.start, f.end, f.rootIdx, f.layerIdx);
                auto start = f.start;
                auto end = f.end;
                auto layerIdx = f.layerIdx;
                while (childChoice.childType == ChildType::Real)
                {
                    mifIntervalIdxs.push_back(childChoice.childIntervalIdx);
                    pending.push(ForestToBuild{
                        .isLeft = false,
                        .rootIdx = childChoice.childIntervalIdx,
                        .layerIdx = layerIdx,
                        .start = childChoice.xPrime,
                        .end = end});
                    if (layerIdx > 0)
                    {
                        pending.push(ForestToBuild{
                            .isLeft = true,
                            .rootIdx = childChoice.childIntervalIdx,
                            .layerIdx = layerIdx - 1,
                            .start = start,
                            .end = childChoice.qPrime});
                    }
                    start = childChoice.qPrime;
                    end = childChoice.xPrime;
                    --layerIdx;
                    childChoice = childChoices.rightChildChoices(start, end, f.rootIdx, layerIdx);
                }
                if(childChoice.childType == ChildType::Dummy)
                {                
                    mifIntervalIdxs.push_back(childChoice.childIntervalIdx);
                    const auto& dummyRoot = intervalModel.getIntervalByIndex(childChoice.childIntervalIdx);    
                    pending.push(ForestToBuild{
                        .isLeft = false,
                        .rootIdx = childChoice.childIntervalIdx,
                        .layerIdx = layerIdx,
                        .start = dummyRoot.Left + 1,
                        .end = end
                    });
                }
            }
        }
    }

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    void Gavril::computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        const auto& allIntervals = intervalModel.getAllIntervals();
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        auto intervalsAtLayer = cg::interval_model_utils::createLayers(intervalModel);

        array4<int> rightForestSizes(intervalModel.end); // 'FR_{w, i}[x, y]' in Gavril's notation.
        array3<int> dummyRightForestSizes(intervalModel.end); // 'FR_{w, i}(r_w, y]' in Gavril's notation.

        const auto& firstLayerIntervals = intervalsAtLayer[0];

        computeRightForestBaseCase(firstLayerIntervals, rightForestSizes, dummyRightForestSizes);

        array4<int> leftForestSizes(intervalModel.end); // 'FL_{w, i}[z, q]' in Gavril's notation.

        computeLeftForestBaseCase(firstLayerIntervals, leftForestSizes);

        array3<int> dummyLeftForestSizes(intervalModel.end); // 'FL_{w, i}(l_w, q]' in Gavril's notation.

        Forests forests{leftForestSizes,dummyLeftForestSizes,rightForestSizes,dummyRightForestSizes};

        ChildChoices childChoices;

        std::vector<cg::data_structures::Interval> cumulativeIntervals; // This is V_i in Gavril's notation. At a given iteration, we set V_i = A_0 U ... A_i
        cumulativeIntervals.insert(cumulativeIntervals.begin(), firstLayerIntervals.begin(), firstLayerIntervals.end());

        std::vector<cg::data_structures::Interval> cumulativeIntervalsOneBehind; // This is V_{i-1} in Gavril's notation.

        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for (int layerIdx = 1; layerIdx < intervalsAtLayer.size(); ++layerIdx)
        {
            const auto &newLayerIntervals = intervalsAtLayer[layerIdx];
            cumulativeIntervalsOneBehind.insert(cumulativeIntervalsOneBehind.begin(), intervalsAtLayer[layerIdx - 1].begin(), intervalsAtLayer[layerIdx - 1].end());
            cumulativeIntervals.insert(cumulativeIntervals.begin(), newLayerIntervals.begin(), newLayerIntervals.end());

            // Evaluate FL, FR for intervals in newLayer
            computeNewIntervalRightForests(layerIdx, newLayerIntervals, cumulativeIntervalsOneBehind, forests);
            computeRightForests(layerIdx, cumulativeIntervals, forests);
            computeLeftForests(layerIdx, cumulativeIntervals, forests);

            computeRightChildChoices(forests, allIntervals, cumulativeIntervals, cumulativeIntervalsOneBehind, childChoices.rightChildChoices, layerIdx); 

        }
    }
}