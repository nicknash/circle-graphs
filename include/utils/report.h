#pragma once

#include <string_view>
#include <span>
#include <iostream>

#include "utils/counters.h"

namespace cg::utils {

// Print a simple report of all counts with associated labels.
template <typename TCounts>
void report(std::string_view name,
            const Counters<TCounts> &counts,
            std::span<const std::string_view> labels,
            std::ostream &out = std::cout)
{
    out << name << "\n";
    for (std::size_t i = 0; i < labels.size(); ++i) {
        out << "  " << labels[i] << ": "
            << counts.Get(static_cast<TCounts>(i)) << "\n";
    }
}

} // namespace cg::utils

