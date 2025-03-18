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
    auto intervals = cg::utils::generateRandomIntervals(10);

    auto intervalRep = cg::data_structures::SimpleIntervalRep(intervals);

    auto mis = cg::mis::Naive::computeMIS(intervalRep);
    
    for(auto i : mis)
    {
        std::cout << std::format("{}", i) << std::endl;
    }
    //cg::mis::Valiente::computeMIS(intervalRep);
    //cg::mis::PureOutputSensitive::tryComputeMIS(intervalRep, 1 + intervals.size());
    return 0;
}