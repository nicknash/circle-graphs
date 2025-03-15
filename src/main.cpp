#include <iostream>

#include "data_structures/interval.h"
#include "data_structures/simple_interval_rep.h"
#include "utils/interval_rep_utils.h"

#include "mis/naive.h"
#include "mis/switching.h"

int main()
{
    auto intervals = cg::utils::generate_random_intervals(1000);

    auto intervalRep = cg::data_structures::SimpleIntervalRep(intervals);

    cg::mis::Naive::computeMIS(intervalRep);
    return 0;
}