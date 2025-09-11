#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

#include <algorithm>

namespace cg::mif
{
    void Gavril::computeRightForestBaseCase(const std::vector<int>& firstLayerEndpoints, const std::vector<cg::data_structures::Interval> firstLayerIntervals, array4<int>& rightForestSizes, array3<int>& dummyRightForestSizes)
    {
        // The base case for rightForestSizes and dummyRightForestSizes:
        // We iterate in decreasing order of right end-point (all layers are sorted increasingly by right end-point)
        for(auto interval : std::views::reverse(firstLayerIntervals)) // 'interval' is 'w' in Gavril's notation
        {
            for(auto y : firstLayerEndpoints) // All end-points y such that: interval.Right < y <= last endpoint at layer 0  
            {
                if(y <= interval.Right)
                {
                    continue;
                }
                int maxDummyForestSize = 0;
                for (const auto &potentialDummyChild : firstLayerIntervals) // Now find the dummy child with largest FR_{v, i}[l_v, y]
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
                    for (const auto &potentialRightChild : firstLayerIntervals)
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

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    void Gavril::computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        auto layers = cg::interval_model_utils::createLayers(intervalModel);

        std::vector<std::vector<int>> endpointsAtLayer;

        std::vector<int> cumulativeEndpoints;
        for (auto &l : layers)
        {
            // Sort the layer by increasing right end-point
            std::sort(l.begin(), l.end(), [](const cg::data_structures::Interval& a, const cg::data_structures::Interval& b) {
              return a.Right < b.Right;
            });
            for (const auto &i : l)
            {
                cumulativeEndpoints.push_back(i.Left);
                cumulativeEndpoints.push_back(i.Right);
            }
            std::sort(cumulativeEndpoints.begin(), cumulativeEndpoints.end());
            endpointsAtLayer.push_back(cumulativeEndpoints);
        }

        const auto &firstLayerEndpoints = endpointsAtLayer[0];
        const auto &firstLayerIntervals = layers[0];

        array4<int> rightForestSizes(intervalModel.end); // 'FR_{w, i}[x, y]' in Gavril's notation.
        array3<int> dummyRightForestSizes(intervalModel.end); // 'FR_{w, i}(r_w, y]' in Gavril's notation.

        computeRightForestBaseCase(firstLayerEndpoints, firstLayerIntervals, rightForestSizes, dummyRightForestSizes);

        auto layersSoFar = layers[0];
        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for (int layerIdx = 1; layerIdx < layers.size(); ++layerIdx)
        {
            // 'currentLayer' is V_k in Gavril's notation, and is the concatenation of this and all previous layers
            const auto &newLayer = layers[layerIdx];
            layersSoFar.insert(layersSoFar.end(), newLayer.begin(), newLayer.end()); // NOTE NICK: after concat no longer in left-to-right order by LEP !

            // Evaluate FL, FR for intervals in newLayer
            // Do the right-to-left scan for all intervals in currentLayer
            // Do the left-to-right scan
            // Calculate the MWIS representative
        }
    }
}