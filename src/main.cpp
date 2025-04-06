#include <iostream>

#include "data_structures/interval.h"
#include "data_structures/simple_interval_rep.h"
#include "utils/interval_rep_utils.h"

#include "mis/naive.h"
#include "mis/valiente.h"
#include "mis/switching.h"
#include "mis/pure_output_sensitive.h"

int main()
{

    for (int i = 0; i < 50; ++i)
    {
        auto intervals = cg::utils::generateRandomIntervals(50 + 100 * i, i);

        auto intervalRep = cg::data_structures::DistinctIntervalRep(intervals);

        auto mis = cg::mis::Naive::computeMIS(intervalRep);
        std::cout << std::format("Naive {}", mis.size()) << std::endl;
        for (auto i : mis)
        {
            //std::cout << std::format("{}", i) << std::endl;
        }
        cg::utils::verifyNoOverlaps(mis);

        auto mis2 = cg::mis::Valiente::computeMIS(intervalRep);
        std::cout << std::format("Valiente {}", mis2.size()) << std::endl;
        for (auto i : mis2)
        {
            // std::cout << std::format("{}", i) << std::endl;
        }
        cg::utils::verifyNoOverlaps(mis2);

        auto mis3 = cg::mis::Switching::computeMIS(intervalRep);
        std::cout << std::format("Switching {}", mis3.size()) << std::endl;
        for (auto i : mis3)
        {
            // std::cout << std::format("{}", i) << std::endl;
        }
        cg::utils::verifyNoOverlaps(mis3);
    
        if(mis.size() != mis2.size() || mis2.size() != mis3.size())
        {
            throw std::runtime_error(std::format("mis.size() = {}, mis2.size() = {}, mis3.size() = {}", mis.size(), mis2.size(), mis3.size()));
        }
    }
    return 0;
}