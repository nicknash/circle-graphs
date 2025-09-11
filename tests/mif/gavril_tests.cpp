#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "mif/gavril.h"
#include "utils/interval_model_utils.h"

#include <algorithm>

static std::vector<int>
collect_first_layer_endpoints(const std::vector<cg::data_structures::Interval>& firstLayer) {
    std::vector<int> eps;
    eps.reserve(2 * firstLayer.size());
    for (const auto& I : firstLayer) {
        eps.push_back(I.Left);
        eps.push_back(I.Right);
    }
    std::sort(eps.begin(), eps.end());
    eps.erase(std::unique(eps.begin(), eps.end()), eps.end());
    return eps;
}

static int expected_count_in_span(const std::vector<cg::data_structures::Interval>& firstLayer, int x, int y) 
{
    if (x >= y) return 0;
    int cnt = 0;
    for (const auto& I : firstLayer) {
        if (x <= I.Left && I.Right <= y) ++cnt;
    }
    return cnt;
}

TEST_CASE("Gavril::computeRightForestBaseCase: 3 disjoint intervals") {
    using cg::data_structures::Interval;

    // Intervals: [0,1], [2,3], [4,5] — all in A0, pairwise disjoint.
    Interval a(0, 1, 0, 1);
    Interval b(2, 3, 1, 1);
    Interval c(4, 5, 2, 1);
    std::vector<Interval> intervals = {a, b, c};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers and take A0
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(layers.size() >= 1);
    auto& A0 = layers[0];

    // Endpoints for A0
    auto A0_eps = collect_first_layer_endpoints(A0);

    // Tables sized by endpoint universe
    cg::mif::array4<int> rightForestSizes(m.end);
    cg::mif::array3<int> dummyRightForestSizes(m.end);

    // Call base case
    cg::mif::Gavril::computeRightForestBaseCase(A0_eps, A0, rightForestSizes, dummyRightForestSizes);

    // Check: x>=y -> zero; i>0 -> zero; and i=0 matches expected counts.
    const int n = static_cast<int>(m.end);
    for (int w = 0; w < n; ++w) { // base case should be independent of w, but filled
        // i=0 checks over all endpoint spans drawn from A0_eps
        for (size_t xi = 0; xi < A0_eps.size(); ++xi) {
            for (size_t yi = 0; yi < A0_eps.size(); ++yi) {
                int x = A0_eps[xi], y = A0_eps[yi];
                int want = expected_count_in_span(A0, x, y);
                CHECK(rightForestSizes(w, 0, x, y) == want);
                CHECK(dummyRightForestSizes(0, x, y) == want);
            }
        }
        // Spot-check that some i>0 remain zero (base case only sets i=0)
        // (Guard in case there is only i=0 in your implementation.)
        if (n > 1) {
            int x = A0_eps.front();
            int y = A0_eps.back();
            CHECK(rightForestSizes(w, 1, x, y) == 0);
        }
    }
}
