
#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

namespace cg::mif
{
    static void computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        const auto& layers = cg::interval_model_utils::createLayers(intervalModel);

        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for(int layerIdx = 1; layerIdx < layers.size(); ++layerIdx)
        {

        }
    }
}