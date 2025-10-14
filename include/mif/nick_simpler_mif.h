#pragma once

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

#include <utility>
#include <vector>

namespace cg::mif
{
    class NickSimplerMif
    {
    public:
        [[nodiscard]] static int computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel);
        [[nodiscard]] static std::pair<int, std::vector<cg::data_structures::Interval>>
        computeMif(const cg::data_structures::DistinctIntervalModel &intervalModel);
    };
}

