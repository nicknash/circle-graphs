#include <iostream>
#include <array>
#include <limits>
#include <string_view>

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/shared_interval_model.h"
#include "utils/interval_model_utils.h"
#include "utils/counters.h"
#include "utils/report.h"

#include "mis/distinct/naive.h"
#include "mis/distinct/valiente.h"
#include "mis/distinct/pure_output_sensitive.h"
#include "mis/distinct/combined_output_sensitive.h"
#include "mis/distinct/simple_implicit_output_sensitive.h"
#include "mis/distinct/implicit_output_sensitive.h"
#include "mis/distinct/lazy_output_sensitive.h"

#include "mis/shared/naive.h"
#include "mis/shared/valiente.h"
#include "mis/shared/pure_output_sensitive.h"
#include "mis/shared/pruned_output_sensitive.h"

int main()
{
    using namespace cg;
    auto intervals = interval_model_utils::generateRandomIntervals(50, 42);
    data_structures::DistinctIntervalModel distinctModel(intervals);
    data_structures::SharedIntervalModel sharedModel(intervals);

    // Distinct endpoint algorithms
    auto misDistinctNaive = mis::distinct::Naive::computeMIS(distinctModel);
    std::size_t expectedSize = misDistinctNaive.size();
    auto misDistinctValiente = mis::distinct::Valiente::computeMIS(distinctModel);
    if (misDistinctValiente.size() != expectedSize) return 1;

    const std::array<std::string_view,3> dsLabels{"StackOuter","StackInner","IntervalOuter"};

    {
        utils::Counters<mis::distinct::PureOutputSensitive::Counts> counts;
        auto mis = mis::distinct::PureOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("distinct pure_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::CombinedOutputSensitive::Counts> counts;
        auto mis = mis::distinct::CombinedOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("distinct combined_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::SimpleImplicitOutputSensitive::Counts> counts;
        auto mis = mis::distinct::SimpleImplicitOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("distinct simple_implicit_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::ImplicitOutputSensitive::Counts> counts;
        auto mis = mis::distinct::ImplicitOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("distinct implicit_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::LazyOutputSensitive::Counts> counts;
        auto mis = mis::distinct::LazyOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("distinct lazy_output_sensitive", counts, dsLabels);
    }

    // Shared endpoint algorithms
    const std::array<std::string_view,2> sdLabels{"InnerLoop","InnerMaxLoop"};
    const std::array<std::string_view,3> ssLabels{"StackOuter","StackInner","IntervalOuter"};

    utils::Counters<mis::shared::Naive::Counts> sharedNaiveCounts;
    auto misSharedNaive = mis::shared::Naive::computeMIS(sharedModel, sharedNaiveCounts);
    if (misSharedNaive.size() != expectedSize) return 1;
    utils::report("shared naive", sharedNaiveCounts, sdLabels);

    utils::Counters<mis::shared::Valiente::Counts> sharedValienteCounts;
    auto misSharedValiente = mis::shared::Valiente::computeMIS(sharedModel, sharedValienteCounts);
    if (misSharedValiente.size() != expectedSize) return 1;
    utils::report("shared valiente", sharedValienteCounts, sdLabels);

    {
        utils::Counters<mis::shared::PureOutputSensitive::Counts> counts;
        auto mis = mis::shared::PureOutputSensitive::tryComputeMIS(sharedModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("shared pure_output_sensitive", counts, ssLabels);
    }
    {
        utils::Counters<mis::shared::PrunedOutputSensitive::Counts> counts;
        auto mis = mis::shared::PrunedOutputSensitive::tryComputeMIS(sharedModel, std::numeric_limits<int>::max(), counts).value();
        if (mis.size() != expectedSize) return 1;
        utils::report("shared pruned_output_sensitive", counts, ssLabels);
    }

    std::cout << "All algorithms agree on MIS size " << expectedSize << "\n";
    return 0;
}
