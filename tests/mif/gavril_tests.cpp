#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "mif/gavril.h"
#include "utils/interval_model_utils.h"

#include <algorithm>

#include <iostream>
#include <format>
#include <map>


static std::vector<int>
collect_first_layer_endpoints(const std::vector<cg::data_structures::Interval>& firstLayer) {
    std::vector<int> eps;
    eps.reserve(2 * firstLayer.size());
    for (const auto& I : firstLayer) {
        eps.push_back(I.Left);
        eps.push_back(I.Right);
    }
    std::sort(eps.begin(), eps.end());
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

    // Build layers and take A0 (sorted by Right inside helper)
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(layers.size() >= 1);
    auto& A0 = layers[0];

    // Endpoints for A0 (must be sorted, contains 0..5 here)
    auto A0_eps = collect_first_layer_endpoints(A0);
    REQUIRE(A0_eps.size() == 6);
    CHECK(std::is_sorted(A0_eps.begin(), A0_eps.end()));

    // Tables sized by endpoint universe (endpoint values used as indices)
    cg::mif::array4<int> FR(m.end);       // FR[x,y,w,0]
    cg::mif::array3<int> FRDummy(m.end);  // FRDummy[y,w,0]

    // Call base case
    cg::mif::Gavril::computeRightForestBaseCase(A0_eps, A0, FR, FRDummy);

    const auto& W0 = A0[0]; // [0,1], index 0
    const auto& W1 = A0[1]; // [2,3], index 1
    const auto& W2 = A0[2]; // [4,5], index 2

    // --- Zeros outside domain: l_w < x <= r_w <= y (i=0) ---
    auto expect_zeros_outside_domain = [&](const Interval& w){
        for (int x : A0_eps) for (int y : A0_eps) {
            bool in_domain = (w.Left < x) && (x <= w.Right) && (w.Right <= y);
            if (!in_domain) {
                CHECK_MESSAGE(FR(x,y,w.Index,0) == 0,
                              "Expected FR to be 0 outside domain for w=", w.Index,
                              " x=", x, " y=", y);
            }
        }
    };
    expect_zeros_outside_domain(W0);
    expect_zeros_outside_domain(W1);
    expect_zeros_outside_domain(W2);

    // --- Expected FR non-zeros (from the known recurrence on disjoint intervals) ---
    // w=0: x=1; FR(1,y) for y=1..5: 1,1,2,2,3
    {
        std::map<int,int> y2v = {{1,1},{2,1},{3,2},{4,2},{5,3}};
        for (int y : A0_eps) {
            int expected = (y2v.count(y) ? y2v[y] : 0);
            CHECK_MESSAGE(FR(1,y,0,0) == expected, "FR(1,", y, ", w=0, i=0) mismatch");
        }
    }
    // w=1: x=3; FR(3,y) for y=3..5: 1,1,2
    {
        std::map<int,int> y2v = {{3,1},{4,1},{5,2}};
        for (int y : A0_eps) {
            int expected = (y2v.count(y) ? y2v[y] : 0);
            CHECK_MESSAGE(FR(3,y,1,0) == expected, "FR(3,", y, ", w=1, i=0) mismatch");
        }
    }
    // w=2: x=5; FR(5,5)=1
    {
        for (int y : A0_eps) {
            int expected = (y == 5 ? 1 : 0);
            CHECK_MESSAGE(FR(5,y,2,0) == expected, "FR(5,", y, ", w=2, i=0) mismatch");
        }
    }

    // --- Dummy table checks: FRDummy(y,w,0) = max FR_{v,0}[l_v+1, y] over v in (r_w,y] ---
    // For w=0 (r_w=1): y=2->0, 3->1, 4->1, 5->2
    {
        std::map<int,int> y2d = {{1,0},{2,0},{3,1},{4,1},{5,2}};
        for (int y : A0_eps) {
            int expected = (y2d.count(y) ? y2d[y] : 0);
            CHECK_MESSAGE(FRDummy(y,0,0) == expected, "FRDummy(y=", y, ", w=0) mismatch");
        }
    }
    // For w=1 (r_w=3): y=4->0, 5->1 (y<=3 -> 0)
    {
        std::map<int,int> y2d = {{3,0},{4,0},{5,1}};
        for (int y : A0_eps) {
            int expected = (y2d.count(y) ? y2d[y] : 0);
            CHECK_MESSAGE(FRDummy(y,1,0) == expected, "FRDummy(y=", y, ", w=1) mismatch");
        }
    }
    // For w=2 (r_w=5): only boundary y=5 -> 0
    {
        for (int y : A0_eps) {
            int expected = (y == 5 ? 0 : 0);
            CHECK_MESSAGE(FRDummy(y,2,0) == expected, "FRDummy(y=", y, ", w=2) mismatch");
        }
    }
}
