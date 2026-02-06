#pragma once

#include <string_view>
#include <span>
#include <iostream>
#include <cmath>

#include "utils/counters.h"

namespace cg::utils {

// Print a simple report of all counts with associated labels.
template <typename TCounts>
void report(std::string_view name,
            const Counters<TCounts> &counts,
            std::span<const std::string_view> labels,
            int instanceSize,
            std::ostream &out = std::cout)
{
    double n1 = instanceSize * instanceSize;
    double n2 = instanceSize * sqrt(instanceSize);

    out << name << "\n";
    for (std::size_t i = 0; i < labels.size(); ++i) 
    {
        long c = counts.Get(static_cast<TCounts>(i)); 
        out << "  " << labels[i] << ":\t\t"
            << c << "\t\t\t" << c / n1 << " (2norm) \t\t\t" << c / n2 << " (1.5norm)\n";
    }
}

} // namespace cg::utils

