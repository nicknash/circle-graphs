#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

#include <algorithm>
#include <stack>

namespace cg::mif
{
    void Gavril::computeRightForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<ForestScore>& rightForestScores, array3<DummyForestScore>& dummyRightForestScores, array4<ChildChoice>& rightChildChoices)
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
                int bestChildIntervalIdx = Invalid;
                for (const auto &potentialDummyChild : firstLayerIntervalsByDecreasingRight) // Now find the dummy child with largest FR_{v, i}[l_v, y]
                {
                    auto isWithinRange = potentialDummyChild.Right <= y; 
                    auto isDummyChild = potentialDummyChild.Left > interval.Right;
                    if (isWithinRange && isDummyChild) // Check that the dummy is within (r_w, y], i.e. within (interval.Right, y]
                    {
                        const auto& dummyChild = potentialDummyChild;
                        const auto dummyForestSizeHere = rightForestScores(dummyChild.Left + 1, y, dummyChild.Index, 0).score;
                        if(dummyForestSizeHere > maxDummyForestSize)
                        {
                            maxDummyForestSize = dummyForestSizeHere;
                            bestChildIntervalIdx = dummyChild.Index;
                        }
                    }
                }
                dummyRightForestScores(y, interval.Index, 0) = DummyForestScore {
                    .score = maxDummyForestSize,
                    .childIntervalIdx = bestChildIntervalIdx
                };
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
                    int bestRealChildIntervalIdx = Invalid;
                    for (const auto &potentialRightChild : firstLayerIntervalsByDecreasingRight)
                    {
                        auto isWithinRange = x <= potentialRightChild.Left && potentialRightChild.Right <= y;
                        auto isRealChild = potentialRightChild.Left < interval.Right && interval.Right < potentialRightChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &rightChild = potentialRightChild; // 'rightChild' is 'v' in Gavril's notation
                            auto xPrime = rightChild.Left + 1;
                            const auto forestSizeHere = rightForestScores(xPrime, y, rightChild.Index, 0).score;
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                                bestRealChildIntervalIdx = rightChild.Index;
                            }
                        }
                    }
                    ForestScore score;
                    auto maxDummy = dummyRightForestScores(y, interval.Index, 0);
                    // We check if y > interval.Right because we only fill dummyRightForestScores when y > interval.Right
                    if(y > interval.Right && maxDummy.score > maxForestSizeFromRealChild)
                    {
                        score = ForestScore {
                            .score = 1 + maxDummy.score,
                            .childIntervalIdx = maxDummy.childIntervalIdx
                        };
                    }
                    else
                    {
                        score = ForestScore{
                            .score = 1 + maxForestSizeFromRealChild,
                            .childIntervalIdx = bestRealChildIntervalIdx
                        };
                    }
                    rightForestScores(x, y, interval.Index, 0) = score;
                    rightChildChoices(x, y, interval.Index, 0) = ChildChoice
                    {
                        .childType = ChildType::None,
                        .score = 0,
                        .qPrime = Invalid,
                        .xPrime = Invalid,
                        .childIntervalIdx = Invalid
                    };
                }
            }
        }
    }


    void Gavril::computeRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& cumulativeIntervals, Forests& forests, array4<ChildChoice>& rightChildChoices)
    {
        if(layerIdx <= 0)
        {
            throw std::invalid_argument(std::format("Should only compute right forests for layers 1 and larger, but layerIdx is {}", layerIdx));
        }
        // Collect all end-points, in increasing order.
        std::vector<int> endpoints;
        for(const auto& interval : cumulativeIntervals)
        {
            endpoints.push_back(interval.Left);
            endpoints.push_back(interval.Right);
        }
        std::sort(endpoints.begin(), endpoints.end());
        // Collect all intervals, ordered by decreasing right-endpoint.
        std::vector<cg::data_structures::Interval> intervalsByDecreasingRight(cumulativeIntervals);
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
                int bestDummyIntervalIdx = Invalid;
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
                forests.dummyRightForestScores(y, interval.Index, layerIdx) = dummyScore;
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
                    int bestChildIntervalIdx = Invalid;
                    auto dummyScore = forests.dummyRightForestScores(y, interval.Index, layerIdx);
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
                                auto leftForestSize = forests.leftForestScores(x, qPrime, rightChild.Index, layerIdx - 1).score;
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
                            .score = 1 + dummyScore.score,
                            .childIntervalIdx = dummyScore.childIntervalIdx
                        };
                    }
                    else
                    {
                        score = ForestScore  
                        {
                            .score = 1 + maxRealChildForestSize,
                            .childIntervalIdx = bestChildIntervalIdx
                        };
                    }
                    forests.rightForestScores(x, y, interval.Index, layerIdx) = score;
                }
            }
        }
    }

    void Gavril::computeNewIntervalRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer, const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer, Forests& forests, array4<ChildChoice>& rightChildChoices)
    {
        if(layerIdx <= 0)
        {
            throw std::invalid_argument(std::format("Should only compute new interval right forests for layers 1 and larger, but layerIdx is {}", layerIdx));
        }
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
                int bestChildIntervalIdx = Invalid;
                for (const auto &potentialDummyChild : allIntervalsBeforeThisLayer) // Now find the dummy child with largest FR_{v, i-1}[l_v, y]
                {
                    auto isWithinRange = potentialDummyChild.Right <= y; 
                    auto isDummyChild = potentialDummyChild.Left > newInterval.Right;
                    if (isWithinRange && isDummyChild) // Check that the dummy is within (r_w, y], i.e. within (interval.Right, y]
                    {
                        const auto& dummyChild = potentialDummyChild;
                        const auto dummyForestSizeHere = forests.rightForestScores(dummyChild.Left + 1, y, dummyChild.Index, previousLayerIdx).score;
                        if(dummyForestSizeHere > maxDummyForestSize)
                        {
                            maxDummyForestSize = dummyForestSizeHere;
                            bestChildIntervalIdx = dummyChild.Index;
                        }
                    }
                }
                forests.dummyRightForestScores(y, newInterval.Index, previousLayerIdx) = DummyForestScore {
                    .score = maxDummyForestSize,
                    .childIntervalIdx = bestChildIntervalIdx
                };
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
                    int bestRealChildIntervalIdx = Invalid;
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
                                auto rightForestSizeHere = forests.rightForestScores(xPrime, y, earlierInterval.Index, previousLayerIdx).score;
                                int sizeHere = 0; 
                                if(previousLayerIdx > 0)
                                {
                                    auto leftForestSizeHere = forests.leftForestScores(x, qPrime, earlierInterval.Index, previousLayerIdx - 1).score;
                                    // Note leftForestSizeHere and rightForestSizeHere both count 'earlierInterval'
                                    // (i.e, FR_{i,v}[x, y] includes v and FL_{i, v}[x, y] includes v) so the '- 1'
                                    // avoids counting 'earlierInterval'  twice.
                                    auto innerSize = rightChildChoices(qPrime, xPrime, newInterval.Index, previousLayerIdx - 1).score;
                                    sizeHere = leftForestSizeHere + rightForestSizeHere + innerSize - 1;
                                }
                                else
                                {
                                    sizeHere = rightForestSizeHere; 
                                }
                                if(sizeHere > maxRealChildForestSize)
                                {
                                    maxRealChildForestSize = sizeHere;
                                    bestRealChildIntervalIdx = earlierInterval.Index;
                                }
                            }
                        }
                    }
                    auto maxDummyForestScore = forests.dummyRightForestScores(y, newInterval.Index, previousLayerIdx);
                    ForestScore score;
                    if(maxDummyForestScore.score > maxRealChildForestSize)
                    {
                        score = ForestScore
                        {
                            .score = 1 + maxDummyForestScore.score,
                            .childIntervalIdx = maxDummyForestScore.childIntervalIdx
                        };
                    }
                    else
                    {
                        score = ForestScore
                        {
                            .score = 1 + maxRealChildForestSize,
                            .childIntervalIdx = bestRealChildIntervalIdx
                        };
                    }
                    forests.rightForestScores(x, y, newInterval.Index, previousLayerIdx) = score;
                }
            }
        }
    }

    void Gavril::computeRightChildChoices(const Forests &forests, const std::vector<cg::data_structures::Interval> allIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervalsOneBehind, array4<ChildChoice> &rightChildChoices, int layerIdx)
    {
        if(layerIdx <= 0)
        {
            throw std::invalid_argument(std::format("Should only compute right child choices for layers 1 and larger, but layerIdx is {}", layerIdx));
        }
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
                    int bestChildIntervalIndex = Invalid;
                    int bestInnerLeft = Invalid; // Gavril calls this z' on page 6
                    int bestInnerRight = Invalid; // Gavril calls this q' on page 6
                    auto dummyScore = forests.dummyRightForestScores(y, interval.Index, layerIdx);
                    auto rightScore = forests.rightForestScores(x, y, interval.Index, layerIdx);

                    ChildType childType;
                    if(dummyScore.childIntervalIdx == Invalid && rightScore.childIntervalIdx == Invalid)
                    {
                        childType = ChildType::None;
                    }
                    else if(dummyScore.childIntervalIdx == Invalid)
                    {
                        childType = ChildType::Real; 
                    }
                    else if(rightScore.childIntervalIdx == Invalid)
                    {
                        childType = ChildType::Dummy;
                    }
                    else
                    {
                        childType = dummyScore.childIntervalIdx == rightScore.childIntervalIdx ? ChildType::Dummy : ChildType::Real;
                    }
                    // We don't compare dummyScore.score to rightScore.score because 'rightScore.score' includes the root interval (i.e., when a dummy 'wins'
                    // we do 1 + dummyScore for it)
                    if (childType == ChildType::Dummy)
                    {
                        bestChildScore = dummyScore.score;
                        bestChildIntervalIndex = dummyScore.childIntervalIdx;
                    }
                    else if(childType == ChildType::Real)
                    {
                        for (const auto &child : cumulativeIntervals)
                        {
                            for (auto innerLeft : endpointsOneBehind) // 'innerLeft' is z' in Gavril's notation on page 6
                            {
                                for (auto innerRight : endpointsOneBehind) // 'innerRight' is q' in Gavril's notation on page 6
                                {
                                    auto isValidInner = interval.Left < x &&
                                                        x <= child.Left &&
                                                        child.Left <= innerLeft &&
                                                        innerLeft < interval.Right &&
                                                        interval.Right < innerRight && 
                                                        innerRight <= child.Right &&
                                                        child.Right <= y;
                                    if (!isValidInner)
                                    {
                                        continue;
                                    }
                                    int scoreHere = forests.leftForestScores(x, innerLeft, child.Index, layerIdx).score +
                                                    forests.rightForestScores(innerRight, y, child.Index, layerIdx).score +
                                                    rightChildChoices(innerLeft, innerRight, interval.Index, layerIdx - 1).score - 1;
                                    if (scoreHere > bestChildScore)
                                    {
                                        bestChildScore = scoreHere;
                                        bestChildIntervalIndex = child.Index;
                                        bestInnerRight = innerRight;
                                        bestInnerLeft = innerLeft;
                                    }
                                }
                            }
                        }
                    }
                    if(childType != ChildType::None && bestChildIntervalIndex != Invalid)
                    {
                        throw std::runtime_error(std::format("Inconsistent child choice: child type is {} and child interval index is {}", childType, bestChildIntervalIndex));
                    }
                    rightChildChoices(x, y, interval.Index, layerIdx) = ChildChoice 
                    {
                        .childType = childType,
                        .score = bestChildScore,
                        .qPrime = bestInnerLeft,
                        .xPrime = bestInnerRight,
                        .childIntervalIdx = bestChildIntervalIndex,
                    };
                }
            }
        }
    }

    void Gavril::computeLeftForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<ForestScore>& leftForestScores, array4<ChildChoice>& leftChildChoices)
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
                    int bestRealChildIntervalIndex = Invalid;
                    for (const auto &potentialLeftChild : firstLayerIntervalsByIncreasingLeft)
                    {
                        auto isWithinRange = z <= potentialLeftChild.Left && potentialLeftChild.Right <= q;
                        auto isRealChild = potentialLeftChild.Left < interval.Left && interval.Left < potentialLeftChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &leftChild = potentialLeftChild; // 'leftChild' is 'v' in Gavril's notation
                            auto qPrime = leftChild.Right - 1;
                            const auto forestSizeHere = leftForestScores(z, qPrime, leftChild.Index, 0).score;
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                                bestRealChildIntervalIndex = leftChild.Index;
                            }
                        }
                    }
                    // There is no dummy to consider against here because at layer 0 there cannot be a left-dummy 
                    // because a left dummy would be contained within 'interval'
                    leftForestScores(z, q, interval.Index, 0) = ForestScore
                    {
                     .score = 1 + maxForestSizeFromRealChild,
                     .childIntervalIdx = bestRealChildIntervalIndex
                    };
                    leftChildChoices(z, q, interval.Index, 0) = ChildChoice {
                        .childType = ChildType::None,
                        .score = 0,
                        .qPrime = Invalid,
                        .xPrime = Invalid,
                        .childIntervalIdx = Invalid
                    };
                }
            }
        }
    }

    void Gavril::computeLeftForests(int layerIdx, const std::vector<cg::data_structures::Interval>& cumulativeIntervals, Forests& forests, array4<ChildChoice>& leftChildChoices)
    {

    }
    
    void Gavril::computeNewIntervalLeftForests(int layerIdx, const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer, const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer, Forests& forests, array4<ChildChoice>& leftChildChoices)
    {

    }
    
    void Gavril::computeLeftChildChoices(const Forests &forests, const std::vector<cg::data_structures::Interval> allIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervalsOneBehind, array4<ChildChoice> &leftChildChoices, int layerIdx)
    {

    }

    void Gavril::constructMif(const cg::data_structures::DistinctIntervalModel intervalModel, int numLayers, const Forests& forests, const ChildChoices& childChoices)
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
        int bestSplit = Invalid;
        int bestIntervalIndex = Invalid;
        int z = 0;
        int y = intervalModel.end - 1;
        for (const auto &w : allIntervals)
        {
            for (auto split = w.Left; split < w.Right; ++split)
            {
                auto scoreHere = forests.leftForestScores(z, split, w.Index, topLayer).score +
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
            if(f.rootIdx < 0 || f.rootIdx >= intervalModel.size || 
               f.layerIdx < 0 || f.layerIdx >= numLayers || 
               f.start < 0 || f.start >= intervalModel.end || 
               f.end < 0 || f.end >= intervalModel.end)
            {
                throw std::runtime_error(
                    std::format("Invalid forest to build: rootIdx={},layerIdx={},start={},end={}. Note interval model end is {}, and size is {} with {} layers",
                f.rootIdx, f.layerIdx, f.start, f.end, intervalModel.end, intervalModel.size, numLayers));
            }
            pending.pop();
            if(f.end < f.start)
            {
                // Not sure if this happens, nothing to do.
                continue;                
            }
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
                else if(childChoice.childType != ChildType::None)
                {
                    throw std::runtime_error(std::format("Unexpected child type {}", childChoice.childType));
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
                else if(childChoice.childType != ChildType::None)
                {
                    throw std::runtime_error(std::format("Unexpected child type {}", childChoice.childType));
                }
            }
        }
    }

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    void Gavril::computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        const ForestScore emptyScore = {
            .score = Invalid,
            .childIntervalIdx = Invalid
        };
        const DummyForestScore emptyDummyScore = {
            .score = Invalid,
            .childIntervalIdx = Invalid
        };
        const ChildChoice emptyChildChoice = {
            .childType = ChildType::Undefined,
            .score = Invalid,
            .qPrime = Invalid,
            .xPrime = Invalid,
            .childIntervalIdx = Invalid
        };

        Forests forests
        {
            .leftForestScores = array4<ForestScore>(intervalModel.end, emptyScore), // 'FL_{w, i}[z, q]' in Gavril's notation.
            .dummyLeftForestScores = array3<DummyForestScore>(intervalModel.end, emptyDummyScore), // 'FL_{w, i}(l_w, q]' in Gavril's notation.
            .rightForestScores = array4<ForestScore>(intervalModel.end, emptyScore), // 'FR_{w, i}[x, y]' in Gavril's notation.
            .dummyRightForestScores = array3<DummyForestScore>(intervalModel.end, emptyDummyScore), // 'FR_{w, i}(r_w, y]' in Gavril's notation.
        };
        ChildChoices childChoices
        {
            .leftChildChoices = array4<ChildChoice>(intervalModel.end, emptyChildChoice), // ul_{w, i}(x, y) in Gavril's notation
            .rightChildChoices = array4<ChildChoice>(intervalModel.end, emptyChildChoice) // ur_{w, i}(x, y) in Gavril's notation
        };
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        auto intervalsAtLayer = cg::interval_model_utils::createLayers(intervalModel);

        const auto& firstLayerIntervals = intervalsAtLayer[0];
        computeRightForestBaseCase(firstLayerIntervals, forests.rightForestScores, forests.dummyRightForestScores, childChoices.rightChildChoices);
        computeLeftForestBaseCase(firstLayerIntervals, forests.leftForestScores, childChoices.leftChildChoices);

        std::vector<cg::data_structures::Interval> cumulativeIntervals; // This is V_i in Gavril's notation. At a given iteration, we set V_i = A_0 U ... A_i
        cumulativeIntervals.insert(cumulativeIntervals.begin(), firstLayerIntervals.begin(), firstLayerIntervals.end());

        std::vector<cg::data_structures::Interval> cumulativeIntervalsOneBehind; // This is V_{i-1} in Gavril's notation.

         const auto& allIntervals = intervalModel.getAllIntervals();
        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for (int layerIdx = 1; layerIdx < intervalsAtLayer.size(); ++layerIdx)
        {
            // First do some book-keeping, so we have:
            // V_i = cumulativeIntervals
            // V_{i - 1} = cumulativeIntervalsOneBehind
            // V_i - V_{i - 1} = newLayerIntervals
            const auto &newLayerIntervals = intervalsAtLayer[layerIdx];
            cumulativeIntervalsOneBehind.insert(cumulativeIntervalsOneBehind.begin(), intervalsAtLayer[layerIdx - 1].begin(), intervalsAtLayer[layerIdx - 1].end());
            cumulativeIntervals.insert(cumulativeIntervals.begin(), newLayerIntervals.begin(), newLayerIntervals.end());

            // Now solve the right-side problems at this layer:

            // Evaluate FR for the intervals in newLayerIntervals, this is described in the paragaph on page 5, "When w \in V_i - V_{i - 1}..."
            computeNewIntervalRightForests(layerIdx, newLayerIntervals, cumulativeIntervalsOneBehind, forests, childChoices.rightChildChoices);
            // Evaluate FR for all w in V_i
            computeRightForests(layerIdx, cumulativeIntervals, forests, childChoices.rightChildChoices);
            // Compute ur_{w, i}(x, y):
            computeRightChildChoices(forests, allIntervals, cumulativeIntervals, cumulativeIntervalsOneBehind, childChoices.rightChildChoices, layerIdx); 
            
            // Now solve the left-side problems at this layer:

            // Evaluate FL for the intervals in newLayerIntervals, this is described in the paragaph on page 5, "When w \in V_i - V_{i - 1}..."
            computeNewIntervalLeftForests(layerIdx, newLayerIntervals, cumulativeIntervalsOneBehind, forests, childChoices.leftChildChoices);
            // Evaluate FL for all w in V_i:
            computeLeftForests(layerIdx, cumulativeIntervals, forests, childChoices.leftChildChoices);
            // Compute ul_{w,i}(x, y):
            computeLeftChildChoices(forests, allIntervals, cumulativeIntervals, cumulativeIntervalsOneBehind, childChoices.leftChildChoices, layerIdx);
        }
    }
}