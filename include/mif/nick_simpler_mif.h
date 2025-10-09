#pragma once

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

namespace cg::mif
{
    class NickSimplerMif
    {
    public:
        [[nodiscard]] static int computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel);
    };
}

