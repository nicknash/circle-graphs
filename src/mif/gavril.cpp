#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

#include <algorithm>

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
        // Collect all intervals, ordered by decreasing right-endpoint.
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

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    void Gavril::computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        auto intervalsAtLayer = cg::interval_model_utils::createLayers(intervalModel);

        array4<int> rightForestSizes(intervalModel.end); // 'FR_{w, i}[x, y]' in Gavril's notation.
        array3<int> dummyRightForestSizes(intervalModel.end); // 'FR_{w, i}(r_w, y]' in Gavril's notation.

        computeRightForestBaseCase(intervalsAtLayer[0], rightForestSizes, dummyRightForestSizes);

        array4<int> leftForestSizes(intervalModel.end); // 'FL_{w, i}[z, q]' in Gavril's notation.

        computeLeftForestBaseCase(intervalsAtLayer[0], leftForestSizes);

        array3<int> dummyLeftForestSizes(intervalModel.end); // 'FL_{w, i}(l_w, q]' in Gavril's notation.

        auto layersSoFar = intervalsAtLayer[0];
        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for (int layerIdx = 1; layerIdx < intervalsAtLayer.size(); ++layerIdx)
        {
            // 'currentLayer' is V_k in Gavril's notation, and is the concatenation of this and all previous layers
            const auto &newLayer = intervalsAtLayer[layerIdx];

            // Evaluate FL, FR for intervals in newLayer
            // Do the right-to-left scan for all intervals in currentLayer
            // Do the left-to-right scan
            // Calculate the MWIS representative
        }
    }
}