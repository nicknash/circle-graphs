#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

#include <algorithm>
#include <stack>

#include <iostream>

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
                dummyRightForestScores(y, interval.Index, 0) = 
                DummyForestScore {
                    .score = maxDummyForestSize,
                    .childIntervalIdx = bestChildIntervalIdx
                };
                std::cout << std::format("DummyFR({},{},0)={}",y,interval.Index,maxDummyForestSize) << std::endl;
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
                    int bestXPrime = Invalid;
                    for (const auto &potentialRightChild : firstLayerIntervalsByDecreasingRight)
                    {
                        auto isWithinRange = x <= potentialRightChild.Left && potentialRightChild.Right <= y;
                        auto isRealChild = potentialRightChild.Left < interval.Right && interval.Right < potentialRightChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &rightChild = potentialRightChild; // 'rightChild' is 'v' in Gavril's notation
                            auto xPrime = interval.Right + 1;// + 1;

                            // N.B. y ---> interval.Right think this through!
                            const auto forestSizeHere = rightForestScores(xPrime, y, rightChild.Index, 0).score;
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                                bestRealChildIntervalIdx = rightChild.Index;
                                bestXPrime = xPrime;
                            }
                        }
                    }
                    ForestScore score;
                    auto childType = ChildType::Undefined;
                    auto innerScore = Invalid;
                    auto maxDummy = dummyRightForestScores(y, interval.Index, 0);
                    // We check if y > interval.Right because we only fill dummyRightForestScores when y > interval.Right
                    if(y > interval.Right && maxDummy.score > maxForestSizeFromRealChild)
                    {
                        childType = ChildType::Dummy;
                        score = ForestScore 
                        {
                            .score = 1 + maxDummy.score,
                            .childIntervalIdx = maxDummy.childIntervalIdx
                        };
                        innerScore = 0;
                    }
                    else if(bestRealChildIntervalIdx != Invalid)
                    {
                        childType = ChildType::Real;
                        score = ForestScore
                        {
                            .score = 1 + maxForestSizeFromRealChild,
                            .childIntervalIdx = bestRealChildIntervalIdx
                        };
                        innerScore = score.score;
                    }
                    else
                    {
                        childType = ChildType::None;
                        score = ForestScore
                        {
                            .score = 1,
                            .childIntervalIdx = Invalid
                        };
                    }
                    auto childChoice = ChildChoice
                        {
                            .childType = childType,
                            .innerScore = 0,//score.score,
                            .qPrime = Invalid, 
                            .xPrime = bestXPrime,
                            .childIntervalIdx =  score.childIntervalIdx
                        };
                    std::cout << std::format("FR({},{},{},0)={} with childType = {}",x,y,interval.Index,score.score, childType) << std::endl;
                    rightForestScores(x, y, interval.Index, 0) = score;
                    rightChildChoices(x, y, interval.Index, 0) = childChoice;
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

                    auto bestQPrime = -1;
                    auto bestXPrime = -1;
                    auto bestLeft = -1;
                    auto bestRight = -1;
                    auto bestInner = -1;

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
                                auto innerSize = rightChildChoices(qPrime, xPrime, interval.Index, layerIdx - 1).innerScore;
                                auto leftForestSize = forests.leftForestScores(x, qPrime, rightChild.Index, layerIdx - 1).score;
                                auto rightForestSize = forests.rightForestScores(xPrime, y, rightChild.Index, layerIdx).score; 
                                int sizeHere = innerSize + leftForestSize + rightForestSize - 1;
                                if(sizeHere > maxRealChildForestSize)
                                {
                                    maxRealChildForestSize = sizeHere;
                                    bestChildIntervalIdx = rightChild.Index;

                                    bestQPrime = qPrime;
                                    bestXPrime = xPrime;
                                    bestLeft = leftForestSize;
                                    bestRight = rightForestSize;
                                    bestInner = innerSize;
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
                    std::cout << std::format("[computeRight] FR({},{},{},{})={} (bestDummy={},bestReal={},dummyChildIdx={},realChildIdx={},bestLeft={},bestRight={},bestInner={},bestQPrime={},bestXPrime={})", 
                        x, y, interval.Index, layerIdx, score.score,dummyScore.score,maxRealChildForestSize,dummyScore.childIntervalIdx,
                        bestChildIntervalIdx,bestLeft,bestRight,bestInner,bestQPrime,bestXPrime) << std::endl;

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
                                    auto innerSize = rightChildChoices(qPrime, xPrime, newInterval.Index, previousLayerIdx - 1).innerScore;
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
                    std::cout << std::format("[newIntervalRight] FR({},{},{},{})={}", x, y, newInterval.Index, previousLayerIdx, score.score) << std::endl;
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
                    int bestChildScore = Invalid;
                    int bestChildIntervalIndex = Invalid;
                    int bestInnerLeft = Invalid; // Gavril calls this z' on page 6
                    int bestInnerRight = Invalid; // Gavril calls this q' on page 6
                    auto dummyScore = forests.dummyRightForestScores(y, interval.Index, layerIdx);
                    auto rightScore = forests.rightForestScores(x, y, interval.Index, layerIdx);

                    auto expectedScore = Invalid;

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
                        expectedScore = 0;//dummyScore.score;
                        bestChildScore = 0;//dummyScore.score;
                        bestChildIntervalIndex = dummyScore.childIntervalIdx;
                    }
                    else if(childType == ChildType::Real)
                    {
                        expectedScore = rightScore.score;
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
                                    //std::cout << 
                                    //std::format("isValidInner(x={},y={},child.Left={},child.Right={},innerLeft={},innerRight={})={}",
                                    //x,y,child.Left,child.Right,innerLeft,innerRight,isValidInner) << std::endl;
                                    if (!isValidInner)
                                    {
                                        continue;
                                    }
                                    auto leftScore = forests.leftForestScores(x, innerLeft, child.Index, layerIdx).score; 
                                    auto rightScore = forests.rightForestScores(innerRight, y, child.Index, layerIdx).score;
                                    auto innerScore = rightChildChoices(innerLeft, innerRight, interval.Index, layerIdx - 1).innerScore; 
                                    int scoreHere = leftScore + rightScore + innerScore - 1; 
                                    
                                    //std::cout << std::format("leftScore={},rightScore={},innerScore={}",leftScore,rightScore,innerScore) << std::endl;
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

                    auto noChildButHaveValidIndex = childType == ChildType::None && bestChildIntervalIndex != Invalid;
                    auto childButInvalidIndex = (childType == ChildType::Dummy || childType == ChildType::Real) && bestChildIntervalIndex == Invalid;
                    if (noChildButHaveValidIndex || childButInvalidIndex)
                    {
                        std::cout << std::format("FR({},{},{},{})={}", x, y, interval.Index, layerIdx, rightScore.score) << std::endl;
                        std::cout << "bestChildIntervalIdx= " << bestChildIntervalIndex << " layerIdx= " << layerIdx << " bestChildScore= " << bestChildScore << " dummyScore.childIntervalIdx= " << dummyScore.childIntervalIdx << " dummyScore=" << dummyScore.score << " rightScore.score=" << rightScore.score << " rightScore child interval idx=" << rightScore.childIntervalIdx << " " << std::endl;
                        throw std::runtime_error(std::format("Inconsistent right child choice: child type is {} and child interval index is {}", childType, bestChildIntervalIndex));
                    }
                    if(expectedScore != bestChildScore)
                    {
                        std::cout << std::format("[NOT EQUAL] FR({},{},{},{})={}", x, y, interval.Index, layerIdx, rightScore.score) << std::endl;
                        std::cout << "[NOT EQUAL] bestChildIntervalIdx= " << bestChildIntervalIndex << " layerIdx= " << layerIdx << " bestChildScore= " << bestChildScore << " dummyScore.childIntervalIdx= " << dummyScore.childIntervalIdx << " dummyScore=" << dummyScore.score << " rightScore.score=" << rightScore.score << " rightScore child interval idx=" << rightScore.childIntervalIdx << " " << std::endl;
                        //throw std::runtime_error(std::format("Expected score was {} but newly computed score is {}", expectedScore, bestChildScore));
                    }

                    rightChildChoices(x, y, interval.Index, layerIdx) = ChildChoice 
                    {
                        .childType = childType,
                        .innerScore = bestChildScore, // is THIS counting w or not ?
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
                    int bestQPrime = Invalid;
                    for (const auto &potentialLeftChild : firstLayerIntervalsByIncreasingLeft)
                    {
                        auto isWithinRange = z <= potentialLeftChild.Left && potentialLeftChild.Right <= q;
                        auto isRealChild = potentialLeftChild.Left < interval.Left && interval.Left < potentialLeftChild.Right;
                        if (isWithinRange && isRealChild)
                        {
                            const auto &leftChild = potentialLeftChild; // 'leftChild' is 'v' in Gavril's notation
                            auto qPrime = interval.Left - 1;
                            const auto forestSizeHere = leftForestScores(z, qPrime, leftChild.Index, 0).score;
                            if (forestSizeHere > maxForestSizeFromRealChild)
                            {
                                maxForestSizeFromRealChild = forestSizeHere;
                                bestRealChildIntervalIndex = leftChild.Index;
                                bestQPrime = qPrime;
                            }
                        }
                    }
                    // There is no dummy to consider against here because at layer 0 there cannot be a left-dummy 
                    // because a left dummy would be contained within 'interval'
                    auto childType = bestRealChildIntervalIndex == -1 ? ChildType::None : ChildType::Real;

                    leftForestScores(z, q, interval.Index, 0) = ForestScore
                    {
                     .score = 1 + maxForestSizeFromRealChild,
                     .childIntervalIdx = bestRealChildIntervalIndex
                    };

                    leftChildChoices(z, q, interval.Index, 0) = ChildChoice {
                        .childType = childType,
                        .innerScore = 0,//1 + maxForestSizeFromRealChild,
                        .qPrime = bestQPrime,
                        .xPrime = Invalid,
                        .childIntervalIdx = bestRealChildIntervalIndex
                    };
                }
            }
        }
    }

void Gavril::computeLeftForests(int layerIdx,
                                const std::vector<cg::data_structures::Interval>& allIntervals,
                                Forests& forests,
                                array4<ChildChoice>& leftChildChoices)
{
    if (layerIdx <= 0)
    {
        throw std::invalid_argument(std::format("Should only compute left forests for layers 1 and larger, but layerIdx is {}", layerIdx));
    }

    // Collect all end-points, in increasing order.
    std::vector<int> endpoints;
    for (const auto& interval : allIntervals)
    {
        endpoints.push_back(interval.Left);
        endpoints.push_back(interval.Right);
    }
    std::sort(endpoints.begin(), endpoints.end());

    // Collect all intervals, ordered by increasing left-endpoint (mirror of right: decreasing right).
    std::vector<cg::data_structures::Interval> intervalsByIncreasingLeft(allIntervals);
    std::sort(intervalsByIncreasingLeft.begin(), intervalsByIncreasingLeft.end(),
              [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
                  return a.Left < b.Left;
              });

    // Iterate in increasing order of left end-point (mirror of right).
    for (auto interval : intervalsByIncreasingLeft) // 'interval' is 'w' in Gavril's notation
    {
        // First, fill dummy-left scores: FL_{w,i}(l_w, q] depends only on q (independent of z).
        for (auto q : endpoints) // All q with: interval.Left < q < interval.Right
        {
            if (q <= interval.Left || q >= interval.Right)
            {
                continue;
            }
            int maxDummyForestSize = 0;
            int bestDummyIntervalIdx = Invalid;

            for (const auto& potentialDummyChild : intervalsByIncreasingLeft)
            {
                // Left-dummy child is fully inside (l_w, q], i.e. l_w < L_v and R_v <= q
                const bool isWithinRange = potentialDummyChild.Right <= q;
                const bool isDummyChild  = potentialDummyChild.Left > interval.Left;

                if (!(isWithinRange && isDummyChild))
                {
                    continue;
                }
                const auto& dummyChild = potentialDummyChild;
                // Symmetric to right: use FL_{v,i}[z', r_v - 1] with z' fixed at v.Left (independent of z of the parent).
                const auto dummyForestSizeHere =
                    forests.leftForestScores(dummyChild.Left, dummyChild.Right - 1, dummyChild.Index, layerIdx).score;

                if (dummyForestSizeHere > maxDummyForestSize)
                {
                    maxDummyForestSize = dummyForestSizeHere;
                    bestDummyIntervalIdx = dummyChild.Index;
                }
            }
            DummyForestScore dummyScore{
                .score = maxDummyForestSize,
                .childIntervalIdx = bestDummyIntervalIdx
            };
            forests.dummyLeftForestScores(q, interval.Index, layerIdx) = dummyScore;
        }

        // Now compute FL_{w,i}[z, q] for all z <= l_w <= q < r_w
        for (auto z : endpoints) // All z such that: first endpoint <= z <= interval.Left
        {
            if (z > interval.Left)
            {
                break;
            }
            for (auto q : endpoints) // All q such that: interval.Left <= q < interval.Right
            {
                if (q < interval.Left)
                {
                    continue;
                }
                if (q >= interval.Right)
                {
                    break;
                }

                int maxRealChildForestSize = 0;
                int bestChildIntervalIdx = Invalid;

                auto dummyScore = forests.dummyLeftForestScores(q, interval.Index, layerIdx);

                for (const auto& potentialLeftChild : intervalsByIncreasingLeft)
                {
                    // Real-left child v must overlap w at l_w and lie within [z, q]:
                    const bool isWithinRange =
                        (z <= potentialLeftChild.Left) && (potentialLeftChild.Right <= q);
                    const bool isRealChild =
                        (potentialLeftChild.Left < interval.Left) && (interval.Left < potentialLeftChild.Right);

                    if (!isWithinRange || !isRealChild)
                    {
                        continue;
                    }

                    const auto& leftChild = potentialLeftChild; // 'leftChild' is 'v' in Gavril's notation

                    // Try all inner split points q' (< l_w) and x' (> l_w) from EP
                    for (auto qPrime : endpoints)
                    {
                        if (qPrime >= interval.Left)
                        {
                            break; // endpoints sorted; once >= l_w we can stop
                        }
                        if (qPrime < leftChild.Left)
                        {
                            continue;
                        }
                        for (auto xPrime : endpoints)
                        {
                            if (xPrime <= interval.Left)
                            {
                                continue;
                            }
                            if (xPrime > leftChild.Right)
                            {
                                break;
                            }

                            // Left part at level i, Right part at level i-1 (dual of right case).
                            const auto leftForestSize =
                                forests.leftForestScores(z, qPrime, leftChild.Index, layerIdx).score;
                            const auto rightForestSize =
                                forests.rightForestScores(xPrime, q, leftChild.Index, layerIdx - 1).score;
                            const auto innerSize =
                                leftChildChoices(qPrime, xPrime, interval.Index, layerIdx - 1).innerScore;

                            const int sizeHere = leftForestSize + rightForestSize + innerSize - 1;
                            if (sizeHere > maxRealChildForestSize)
                            {
                                maxRealChildForestSize = sizeHere;
                                bestChildIntervalIdx = leftChild.Index;
                            }
                        }
                    }
                }

                ForestScore score;
                if (dummyScore.score > maxRealChildForestSize)
                {
                    score = ForestScore{
                        .score = 1 + dummyScore.score,
                        .childIntervalIdx = dummyScore.childIntervalIdx
                    };
                }
                else
                {
                    score = ForestScore{
                        .score = 1 + maxRealChildForestSize,
                        .childIntervalIdx = bestChildIntervalIdx
                    };
                }
                forests.leftForestScores(z, q, interval.Index, layerIdx) = score;
            }
        }
    }
}

void Gavril::computeNewIntervalLeftForests(int layerIdx,
                                           const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer,
                                           const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer,
                                           Forests& forests,
                                           array4<ChildChoice>& leftChildChoices)
{
    if (layerIdx <= 0)
    {
        throw std::invalid_argument(std::format("Should only compute new interval left forests for layers 1 and larger, but layerIdx is {}", layerIdx));
    }

    // All endpoints from V_{i-1}
    std::vector<int> allEndpoints;
    for (const auto& interval : allIntervalsBeforeThisLayer)
    {
        allEndpoints.push_back(interval.Left);
        allEndpoints.push_back(interval.Right);
    }
    std::sort(allEndpoints.begin(), allEndpoints.end());

    const auto previousLayerIdx = layerIdx - 1;

    for (const auto& newInterval : newIntervalsAtThisLayer) // w ∈ V_i − V_{i−1}
    {
        // 1) Fill FL_{w, i-1}(l_w, q] (dummy-left) for this w
        for (auto q : allEndpoints)
        {
            if (q <= newInterval.Left || q >= newInterval.Right)
            {
                continue;
            }
            int maxDummyForestSize = 0;
            int bestChildIntervalIdx = Invalid;

            for (const auto& potentialDummyChild : allIntervalsBeforeThisLayer)
            {
                const bool isWithinRange = potentialDummyChild.Right <= q;
                const bool isDummyChild  = potentialDummyChild.Left > newInterval.Left;

                if (!isWithinRange || !isDummyChild)
                {
                    continue;
                }
                const auto& dummyChild = potentialDummyChild;
                // At level i-1 (since w is newly added at i), mirror of right-new:
                const auto dummyForestSizeHere =
                    forests.leftForestScores(dummyChild.Left, dummyChild.Right - 1, dummyChild.Index, previousLayerIdx).score;

                if (dummyForestSizeHere > maxDummyForestSize)
                {
                    maxDummyForestSize = dummyForestSizeHere;
                    bestChildIntervalIdx = dummyChild.Index;
                }
            }
            forests.dummyLeftForestScores(q, newInterval.Index, previousLayerIdx) = DummyForestScore{
                .score = maxDummyForestSize,
                .childIntervalIdx = bestChildIntervalIdx
            };
        }

        // 2) Compute FL_{w, i-1}[z, q] for all valid z, q
        for (auto z : allEndpoints)
        {
            if (z > newInterval.Left)
            {
                break;
            }
            for (auto q : allEndpoints)
            {
                if (q < newInterval.Left)
                {
                    continue;
                }
                if (q >= newInterval.Right)
                {
                    break;
                }

                int maxRealChildForestSize = 0;
                int bestRealChildIntervalIdx = Invalid;

                for (const auto& earlierInterval : allIntervalsBeforeThisLayer)
                {
                    const bool isWithinRange =
                        (z <= earlierInterval.Left) && (earlierInterval.Right <= q);
                    const bool isLeftChild =
                        (earlierInterval.Left < newInterval.Left) && (newInterval.Left < earlierInterval.Right);

                    if (!isWithinRange || !isLeftChild)
                    {
                        continue;
                    }

                    // Try inner split points q' (< l_w) and x' (> l_w)
                    for (auto qPrime : allEndpoints)
                    {
                        if (qPrime >= newInterval.Left)
                        {
                            break;
                        }
                        if (qPrime < earlierInterval.Left)
                        {
                            continue;
                        }
                        for (auto xPrime : allEndpoints)
                        {
                            if (xPrime <= newInterval.Left)
                            {
                                continue;
                            }
                            if (xPrime > earlierInterval.Right)
                            {
                                break;
                            }

                            // At new-interval stage: mirror right code’s level usage.
                            int sizeHere = 0;
                            if (previousLayerIdx > 0)
                            {
                                const auto leftForestSizeHere =
                                    forests.leftForestScores(z, qPrime, earlierInterval.Index, previousLayerIdx).score;
                                const auto rightForestSizeHere =
                                    forests.rightForestScores(xPrime, q, earlierInterval.Index, previousLayerIdx - 1).score;
                                const auto innerSize =
                                    leftChildChoices(qPrime, xPrime, newInterval.Index, previousLayerIdx - 1).innerScore;

                                // left + right include 'earlierInterval' both; subtract 1 once (mirror of right).
                                sizeHere = leftForestSizeHere + rightForestSizeHere + innerSize - 1;
                            }
                            else
                            {
                                // Base of this stage: only the left part survives (dual of right using right-only).
                                sizeHere = forests.leftForestScores(z, qPrime, earlierInterval.Index, previousLayerIdx).score;
                            }

                            if (sizeHere > maxRealChildForestSize)
                            {
                                maxRealChildForestSize = sizeHere;
                                bestRealChildIntervalIdx = earlierInterval.Index;
                            }
                        }
                    }
                }

                const auto maxDummyForestScore =
                    forests.dummyLeftForestScores(q, newInterval.Index, previousLayerIdx);

                ForestScore score;
                if (maxDummyForestScore.score > maxRealChildForestSize)
                {
                    score = ForestScore{
                        .score = 1 + maxDummyForestScore.score,
                        .childIntervalIdx = maxDummyForestScore.childIntervalIdx
                    };
                }
                else
                {
                    score = ForestScore{
                        .score = 1 + maxRealChildForestSize,
                        .childIntervalIdx = bestRealChildIntervalIdx
                    };
                }

                forests.leftForestScores(z, q, newInterval.Index, previousLayerIdx) = score;
            }
        }
    }
}

void Gavril::computeLeftChildChoices(const Forests& forests,
                                     const std::vector<cg::data_structures::Interval> allIntervals,
                                     const std::vector<cg::data_structures::Interval>& cumulativeIntervals,
                                     const std::vector<cg::data_structures::Interval>& cumulativeIntervalsOneBehind,
                                     array4<ChildChoice>& leftChildChoices,
                                     int layerIdx)
{
    if (layerIdx <= 0)
    {
        throw std::invalid_argument(std::format("Should only compute left child choices for layers 1 and larger, but layerIdx is {}", layerIdx));
    }

    std::vector<int> endpoints;
    for (const auto& interval : cumulativeIntervals)
    {
        endpoints.push_back(interval.Left);
        endpoints.push_back(interval.Right);
    }
    std::sort(endpoints.begin(), endpoints.end());

    std::vector<int> endpointsOneBehind;
    for (const auto& interval : cumulativeIntervalsOneBehind)
    {
        endpointsOneBehind.push_back(interval.Left);
        endpointsOneBehind.push_back(interval.Right);
    }
    std::sort(endpointsOneBehind.begin(), endpointsOneBehind.end());

    for (const auto& interval : allIntervals)
    {
        for (auto z : endpoints)
        {
            for (auto q : endpoints)
            {
                int bestChildScore = 0;
                int bestChildIntervalIndex = Invalid;
                int bestInnerQ = Invalid;  // this will be q' (< l_w)
                int bestInnerX = Invalid;  // this will be x' (> l_w)

                const auto dummyScore = forests.dummyLeftForestScores(q, interval.Index, layerIdx);
                const auto leftScore  = forests.leftForestScores(z, q, interval.Index, layerIdx);

                ChildType childType;
                if (dummyScore.childIntervalIdx == Invalid && leftScore.childIntervalIdx == Invalid)
                {
                    childType = ChildType::None;
                }
                else if (dummyScore.childIntervalIdx == Invalid)
                {
                    childType = ChildType::Real;
                }
                else if (leftScore.childIntervalIdx == Invalid)
                {
                    childType = ChildType::Dummy;
                }
                else
                {
                    childType = (dummyScore.childIntervalIdx == leftScore.childIntervalIdx) ? ChildType::Dummy
                                                                                           : ChildType::Real;
                }

                if (childType == ChildType::Dummy)
                {
                    bestChildScore = 0;//dummyScore.score;
                    bestChildIntervalIndex = dummyScore.childIntervalIdx;
                }
                else if (childType == ChildType::Real)
                {
                    for (const auto& child : cumulativeIntervals)
                    {
                        for (auto qPrime : endpointsOneBehind) // q' (< l_w)
                        {
                            if (qPrime >= interval.Left) break;
                            for (auto xPrime : endpointsOneBehind) // x' (> l_w)
                            {
                                if (xPrime <= interval.Left) continue;

                                // Validity mirror of right: z <= child.Left <= q' < l_w < x' <= child.Right <= q
                                const bool isValidInner =
                                    (z <= child.Left) &&
                                    (child.Left <= qPrime) &&
                                    (qPrime < interval.Left) &&
                                    (interval.Left < xPrime) &&
                                    (xPrime <= child.Right) &&
                                    (child.Right <= q);

                                if (!isValidInner) continue;

                                const int scoreHere =
                                    forests.leftForestScores(z, qPrime, child.Index, layerIdx).score +
                                    forests.rightForestScores(xPrime, q, child.Index, layerIdx).score +
                                    leftChildChoices(qPrime, xPrime, interval.Index, layerIdx - 1).innerScore - 1;

                                if (scoreHere > bestChildScore)
                                {
                                    bestChildScore = scoreHere;
                                    bestChildIntervalIndex = child.Index;
                                    bestInnerQ = qPrime;
                                    bestInnerX = xPrime;
                                }
                            }
                        }
                    }
                }

                auto noChildButHaveValidIndex = childType == ChildType::None && bestChildIntervalIndex != Invalid;
                auto childButInvalidIndex = (childType == ChildType::Dummy || childType == ChildType::Real) && bestChildIntervalIndex == Invalid;
                if (noChildButHaveValidIndex || childButInvalidIndex)
                {
                    throw std::runtime_error(std::format("Inconsistent left child choice: child type is {} and child interval index is {}", childType, bestChildIntervalIndex));
                }

                leftChildChoices(z, q, interval.Index, layerIdx) = ChildChoice{
                    .childType = childType,
                    .innerScore = bestChildScore,
                    .qPrime = bestInnerQ,    // q'
                    .xPrime = bestInnerX,    // x'
                    .childIntervalIdx = bestChildIntervalIndex
                };
            }
        }
    }
}

    std::vector<int> Gavril::constructMif(const cg::data_structures::DistinctIntervalModel& intervalModel, int numLayers, const Forests& forests, const ChildChoices& childChoices)
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
        int bestLeft = 0;
        int bestRight = 0;
        int z = 0;
        int y = intervalModel.end - 1;
        for (const auto &w : allIntervals)
        {
            for (auto split = w.Left; split < w.Right; ++split)
            {
                auto leftScore = forests.leftForestScores(z, split, w.Index, topLayer).score;
                auto rightScore = forests.rightForestScores(split + 1, y, w.Index, topLayer).score; 
                auto scoreHere = leftScore + rightScore - 1;
                if (scoreHere > bestScore)
                {
                    bestLeft = leftScore;
                    bestRight = rightScore;
                    bestScore = scoreHere;
                    bestSplit = split;
                    bestIntervalIndex = w.Index;
                }
            }
        }
        std::cout << std::format("bestSplit={},bestScore={},bestIntervalIndex={},bestLeft={},bestRight={}",bestSplit,bestScore,bestIntervalIndex,bestLeft,bestRight) << std::endl;

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

                        start = childChoice.qPrime;
                        end = childChoice.xPrime;
                        --layerIdx;
                        childChoice = childChoices.leftChildChoices(start, end, f.rootIdx, layerIdx);
                    }
                    else
                    {
                        childChoice.childType = ChildType::None;
                    }
           
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
                        pending.push(
                            ForestToBuild{
                                .isLeft = true,
                                .rootIdx = childChoice.childIntervalIdx,
                                .layerIdx = layerIdx - 1,
                                .start = start,
                                .end = childChoice.qPrime});
                        start = childChoice.qPrime;
                        end = childChoice.xPrime;
                        --layerIdx;
                        childChoice = childChoices.rightChildChoices(start, end, f.rootIdx, layerIdx);
                    }
                    else
                    {
                        childChoice.childType = ChildType::None;
                    }
                }
                if(childChoice.childType == ChildType::Dummy)
                {                
                    if(childChoice.childIntervalIdx < 0 || childChoice.childIntervalIdx >= intervalModel.size)
                    {
                        throw std::runtime_error(std::format("{}", childChoice.childIntervalIdx));
                    }
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
        if(mifIntervalIdxs.size() != bestScore)
        {
            throw std::runtime_error(std::format("Induced forest size of {} doesn't match computed forest size of {}", mifIntervalIdxs.size(), bestScore));
        }
        return mifIntervalIdxs;
    }

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    std::vector<cg::data_structures::Interval> Gavril::computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        const ForestScore emptyScore = {
            .score = 0,
            .childIntervalIdx = Invalid
        };
        const DummyForestScore emptyDummyScore = {
            .score = 0,
            .childIntervalIdx = Invalid
        };
        const ChildChoice emptyChildChoice = {
            .childType = ChildType::Undefined,
            .innerScore = 0,
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
        auto mifIntervalIdxs = constructMif(intervalModel, intervalsAtLayer.size(), forests, childChoices);
        std::vector<cg::data_structures::Interval> mifIntervals;
        for(auto idx : mifIntervalIdxs)
        {
            if(idx < 0 || idx >= intervalModel.size)
            {
                throw std::runtime_error(std::format("Invalid interval index {}", idx));
            }
            mifIntervals.push_back(intervalModel.getIntervalByIndex(idx));
        }
        return mifIntervals;
    }
}