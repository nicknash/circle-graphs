
#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

namespace cg::mif
{
    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    static void computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        const auto& layers = cg::interval_model_utils::createLayers(intervalModel);

        const auto& firstLayer = layers[0];
        // TODONICK: Implement the base cases.


        auto layersSoFar = firstLayer;
        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for(int layerIdx = 1; layerIdx < layers.size(); ++layerIdx)
        {
            // 'currentLayer' is V_k in Gavril's notation, and is the concatenation of this and all previous layers
            const auto& newLayer = layers[layerIdx];
            layersSoFar.insert(layersSoFar.end(), newLayer.begin(), newLayer.end());
        
            // Evaluate FL, FR for intervals in newLayer
            // Do the right-to-left scan for all intervals in currentLayer
            // Do the left-to-right scan
            // Calculate the MWIS representative
        }
    }
}