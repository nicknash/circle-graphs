#include <iostream>
#include <array>
#include <limits>
#include <string_view>
#include <vector>
#include <utility>

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

    std::vector<std::pair<std::string_view, std::size_t>> results;

    // Distinct endpoint algorithms
    auto misDistinctNaive = mis::distinct::Naive::computeMIS(distinctModel);
    std::size_t expectedSize = misDistinctNaive.size();
    results.emplace_back("distinct naive", misDistinctNaive.size());

    auto misDistinctValiente = mis::distinct::Valiente::computeMIS(distinctModel);
    results.emplace_back("distinct valiente", misDistinctValiente.size());

    const std::array<std::string_view,3> dsLabels{"StackOuter","StackInner","IntervalOuter"};

    {
        utils::Counters<mis::distinct::PureOutputSensitive::Counts> counts;
        auto mis = mis::distinct::PureOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("distinct pure_output_sensitive", mis.size());
        utils::report("distinct pure_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::CombinedOutputSensitive::Counts> counts;
        auto mis = mis::distinct::CombinedOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("distinct combined_output_sensitive", mis.size());
        utils::report("distinct combined_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::SimpleImplicitOutputSensitive::Counts> counts;
        auto mis = mis::distinct::SimpleImplicitOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("distinct simple_implicit_output_sensitive", mis.size());
        utils::report("distinct simple_implicit_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::ImplicitOutputSensitive::Counts> counts;
        auto mis = mis::distinct::ImplicitOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("distinct implicit_output_sensitive", mis.size());
        utils::report("distinct implicit_output_sensitive", counts, dsLabels);
    }
    {
        utils::Counters<mis::distinct::LazyOutputSensitive::Counts> counts;
        auto mis = mis::distinct::LazyOutputSensitive::tryComputeMIS(distinctModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("distinct lazy_output_sensitive", mis.size());
        utils::report("distinct lazy_output_sensitive", counts, dsLabels);
    }

    // Shared endpoint algorithms
    const std::array<std::string_view,2> sdLabels{"InnerLoop","InnerMaxLoop"};
    const std::array<std::string_view,3> ssLabels{"StackOuter","StackInner","IntervalOuter"};

    utils::Counters<mis::shared::Naive::Counts> sharedNaiveCounts;
    auto misSharedNaive = mis::shared::Naive::computeMIS(sharedModel, sharedNaiveCounts);
    results.emplace_back("shared naive", misSharedNaive.size());
    utils::report("shared naive", sharedNaiveCounts, sdLabels);

    utils::Counters<mis::shared::Valiente::Counts> sharedValienteCounts;
    auto misSharedValiente = mis::shared::Valiente::computeMIS(sharedModel, sharedValienteCounts);
    results.emplace_back("shared valiente", misSharedValiente.size());
    utils::report("shared valiente", sharedValienteCounts, sdLabels);

    {
        utils::Counters<mis::shared::PureOutputSensitive::Counts> counts;
        auto mis = mis::shared::PureOutputSensitive::tryComputeMIS(sharedModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("shared pure_output_sensitive", mis.size());
        utils::report("shared pure_output_sensitive", counts, ssLabels);
    }
    {
        utils::Counters<mis::shared::PrunedOutputSensitive::Counts> counts;
        auto mis = mis::shared::PrunedOutputSensitive::tryComputeMIS(sharedModel, std::numeric_limits<int>::max(), counts).value();
        results.emplace_back("shared pruned_output_sensitive", mis.size());
        utils::report("shared pruned_output_sensitive", counts, ssLabels);
    }

    bool allMatch = true;
    for (const auto& [name, size] : results)
    {
        std::cout << name << " MIS size: " << size << "\n";
        if (size != expectedSize)
        {
            allMatch = false;
        }
    }

    if (allMatch)
    {
        std::cout << "All algorithms agree on MIS size " << expectedSize << "\n";
        return 0;
    }

    std::cout << "Algorithms disagree on MIS size. Expected " << expectedSize << "\n";
    for (const auto& [name, size] : results)
    {
        if (size != expectedSize)
        {
            std::cout << " - " << name << " size: " << size << "\n";
        }
    }
    return 1;
}
