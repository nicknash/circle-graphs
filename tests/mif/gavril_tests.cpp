#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "mif/gavril.h"
#include "utils/interval_model_utils.h"

#include <algorithm>

#include <format>
#include <map>

namespace {

using ForestScore      = cg::mif::Gavril::ForestScore;
using DummyForestScore = cg::mif::Gavril::DummyForestScore;
using ChildChoice      = cg::mif::Gavril::ChildChoice;

constexpr ForestScore kZeroForestScore{0, cg::mif::Gavril::Invalid};
constexpr DummyForestScore kZeroDummyForestScore{0, cg::mif::Gavril::Invalid};
constexpr ChildChoice kUnsetChildChoice{
    cg::mif::ChildType::Undefined,
    0,
    cg::mif::Gavril::Invalid,
    cg::mif::Gavril::Invalid,
    cg::mif::Gavril::Invalid
};

std::vector<int>
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

[[maybe_unused]] int expected_count_in_span(const std::vector<cg::data_structures::Interval>& firstLayer, int x, int y)
{
    if (x >= y) return 0;
    int cnt = 0;
    for (const auto& I : firstLayer) {
        if (x <= I.Left && I.Right <= y) ++cnt;
    }
    return cnt;
}

int forest_score(const cg::mif::array4<ForestScore>& table, int x, int y, int intervalIndex, int layer = 0)
{
    return table(x, y, intervalIndex, layer).score;
}

int forest_score(const cg::mif::array4<ForestScore>& table, int x, int y, const cg::data_structures::Interval& interval, int layer = 0)
{
    return forest_score(table, x, y, interval.Index, layer);
}

int dummy_score(const cg::mif::array3<DummyForestScore>& table, int y, int intervalIndex, int layer = 0)
{
    return table(y, intervalIndex, layer).score;
}

int dummy_score(const cg::mif::array3<DummyForestScore>& table, int y, const cg::data_structures::Interval& interval, int layer = 0)
{
    return dummy_score(table, y, interval.Index, layer);
}

} // namespace
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
    cg::mif::array4<ForestScore> FR(m.end, kZeroForestScore);       // FR[x,y,w,0]
    cg::mif::array3<DummyForestScore> FRDummy(m.end, kZeroDummyForestScore);  // FRDummy[y,w,0]
    cg::mif::array4<ChildChoice> FRChoices(m.end, kUnsetChildChoice);

    // Call base case
    cg::mif::Gavril::computeRightForestBaseCase(A0, FR, FRDummy, FRChoices);

    const auto& W0 = A0[0]; // [0,1], index 0
    const auto& W1 = A0[1]; // [2,3], index 1
    const auto& W2 = A0[2]; // [4,5], index 2

    // --- Zeros outside domain: l_w < x <= r_w <= y (i=0) ---
    auto expect_zeros_outside_domain = [&](const Interval& w){
        for (int x : A0_eps) for (int y : A0_eps) {
            bool in_domain = (w.Left < x) && (x <= w.Right) && (w.Right <= y);
            if (!in_domain) {
                CHECK_MESSAGE(forest_score(FR, x, y, w) == 0,
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
            CHECK_MESSAGE(forest_score(FR, 1, y, 0) == expected, "FR(1,", y, ", w=0, i=0) mismatch");
        }
    }
    // w=1: x=3; FR(3,y) for y=3..5: 1,1,2
    {
        std::map<int,int> y2v = {{3,1},{4,1},{5,2}};
        for (int y : A0_eps) {
            int expected = (y2v.count(y) ? y2v[y] : 0);
            CHECK_MESSAGE(forest_score(FR, 3, y, 1) == expected, "FR(3,", y, ", w=1, i=0) mismatch");
        }
    }
    // w=2: x=5; FR(5,5)=1
    {
        for (int y : A0_eps) {
            int expected = (y == 5 ? 1 : 0);
            CHECK_MESSAGE(forest_score(FR, 5, y, 2) == expected, "FR(5,", y, ", w=2, i=0) mismatch");
        }
    }

    // --- Dummy table checks: FRDummy(y,w,0) = max FR_{v,0}[l_v+1, y] over v in (r_w,y] ---
    // For w=0 (r_w=1): y=2->0, 3->1, 4->1, 5->2
    {
        std::map<int,int> y2d = {{1,0},{2,0},{3,1},{4,1},{5,2}};
        for (int y : A0_eps) {
            int expected = (y2d.count(y) ? y2d[y] : 0);
            CHECK_MESSAGE(dummy_score(FRDummy, y, 0) == expected, "FRDummy(y=", y, ", w=0) mismatch");
        }
    }
    // For w=1 (r_w=3): y=4->0, 5->1 (y<=3 -> 0)
    {
        std::map<int,int> y2d = {{3,0},{4,0},{5,1}};
        for (int y : A0_eps) {
            int expected = (y2d.count(y) ? y2d[y] : 0);
            CHECK_MESSAGE(dummy_score(FRDummy, y, 1) == expected, "FRDummy(y=", y, ", w=1) mismatch");
        }
    }
    // For w=2 (r_w=5): only boundary y=5 -> 0
    {
        for (int y : A0_eps) {
            int expected = (y == 5 ? 0 : 0);
            CHECK_MESSAGE(dummy_score(FRDummy, y, 2) == expected, "FRDummy(y=", y, ", w=2) mismatch");
        }
    }
}

TEST_CASE("Gavril::computeRightForestBaseCase: real-child transitions exist, but only one layer") {
    using cg::data_structures::Interval;

    // Intervals (endpoints 0..5):
    // A=[0,3] (Index=0), B=[2,5] (Index=1), C=[1,4] (Index=2).
    Interval A(0, 3, 0, 1);
    Interval B(2, 5, 1, 1);
    Interval C(1, 4, 2, 1);
    std::vector<Interval> intervals = {A, B, C};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers and take A0
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(!layers.empty());
    auto& A0 = layers[0];

    // Sort by increasing Right (to match production)
    std::sort(A0.begin(), A0.end(),
              [](const Interval& a, const Interval& b){ return a.Right < b.Right; });

    // Endpoints for A0 (use your helper; replace with local impl if needed)
    auto A0_eps = collect_first_layer_endpoints(A0);
    REQUIRE(A0_eps.size() == 6);
    CHECK(std::is_sorted(A0_eps.begin(), A0_eps.end()));

    // Tables sized by endpoint *count* (0..5 inclusive ⇒ m.end must be 6; if m.end is max endpoint, use m.end+1)
    cg::mif::array4<ForestScore> FR(m.end, kZeroForestScore);
    cg::mif::array3<DummyForestScore> FRDummy(m.end, kZeroDummyForestScore);
    cg::mif::array4<ChildChoice> FRChoices(m.end, kUnsetChildChoice);

    // Compute base case
    cg::mif::Gavril::computeRightForestBaseCase(A0, FR, FRDummy, FRChoices);

  
    auto in_domain = [](const Interval& w, int x, int y){
        return (w.Left < x) && (x <= w.Right) && (w.Right <= y);
    };

    // 1) Zeros outside i=0 domain
    auto expect_zeros_outside = [&](const Interval& w){
        for (int x : A0_eps) for (int y : A0_eps)
            if (!in_domain(w,x,y))
                CHECK_MESSAGE(forest_score(FR, x, y, w) == 0,
                              "FR not zero outside domain for w=", w.Index, " x=", x, " y=", y);
    };
    expect_zeros_outside(A);
    expect_zeros_outside(B);
    expect_zeros_outside(C);

    // 2) Dummy tables: none exist in this configuration
    for (int y : A0_eps) {
        CHECK(dummy_score(FRDummy, y, A) == 0);
        CHECK(dummy_score(FRDummy, y, B) == 0);
        CHECK(dummy_score(FRDummy, y, C) == 0);
    }

    // 3) Expected FR with real-child contributions
    // B=[2,5]: no real children; only y=5 in domain
    CHECK(forest_score(FR, 3, 5, B) == 1);
    CHECK(forest_score(FR, 4, 5, B) == 1);
    CHECK(forest_score(FR, 5, 5, B) == 1);

    // C=[1,4]: real child is B (2<4<5). Only x=2 and y=5 allow B; else 1.
    CHECK(forest_score(FR, 2, 4, C) == 1);
    CHECK(forest_score(FR, 2, 5, C) == 2); // 1 + FR_B(3,5)=2
    CHECK(forest_score(FR, 3, 4, C) == 1);
    CHECK(forest_score(FR, 3, 5, C) == 1);
    CHECK(forest_score(FR, 4, 4, C) == 1);
    CHECK(forest_score(FR, 4, 5, C) == 1);

    // A=[0,3]: real children are C and B.
    CHECK(forest_score(FR, 1, 3, A) == 1);
    CHECK(forest_score(FR, 1, 4, A) == 2); // via C: 1 + FR_C(2,4)=2
    CHECK(forest_score(FR, 1, 5, A) == 3); // via max(C->2, B->1) + 1
    CHECK(forest_score(FR, 2, 3, A) == 1);
    CHECK(forest_score(FR, 2, 4, A) == 1);
    CHECK(forest_score(FR, 2, 5, A) == 2); // via B: 1 + FR_B(3,5)=2
    CHECK(forest_score(FR, 3, 3, A) == 1);
    CHECK(forest_score(FR, 3, 4, A) == 1);
    CHECK(forest_score(FR, 3, 5, A) == 1);
}

TEST_CASE("Gavril::computeRightForestBaseCase: real-child transitions exist, nested inside two outers") {
    using cg::data_structures::Interval;

    // Inner A0 intervals (endpoints 2..7):
    // A=[2,5] (Index=0), B=[4,7] (Index=1), C=[3,6] (Index=2).
    Interval A(2, 5, 0, 1);
    Interval B(4, 7, 1, 1);
    Interval C(3, 6, 2, 1);

    // Two outers that contain all three inners (do not belong to A0):
    Interval E(1, 8, 3, 1);
    Interval D(0, 9, 4, 1);

    std::vector<Interval> intervals = {A, B, C, E, D};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers and take A0
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(!layers.empty());
    auto& A0 = layers[0];

    // Sort by increasing Right to match production code
    std::sort(A0.begin(), A0.end(),
              [](const Interval& a, const Interval& b){ return a.Right < b.Right; });

    // Endpoints for A0 only (expected {2,3,4,5,6,7})
    auto A0_eps = collect_first_layer_endpoints(A0);
    REQUIRE(A0_eps.size() == 6);
    CHECK(std::is_sorted(A0_eps.begin(), A0_eps.end()));

    // DP tables keyed by endpoint values; if m.end is "max endpoint", use m.end+1
    cg::mif::array4<ForestScore> FR(m.end, kZeroForestScore);
    cg::mif::array3<DummyForestScore> FRDummy(m.end, kZeroDummyForestScore);
    cg::mif::array4<ChildChoice> FRChoices(m.end, kUnsetChildChoice);

    // Compute base case over A0
    cg::mif::Gavril::computeRightForestBaseCase(A0, FR, FRDummy, FRChoices);

    auto in_domain = [](const Interval& w, int x, int y){
        return (w.Left < x) && (x <= w.Right) && (w.Right <= y);
    };

    // --- 1) Zeros outside i=0 domain ---
    auto expect_zeros_outside = [&](const Interval& w){
        for (int x : A0_eps) for (int y : A0_eps) {
            if (!in_domain(w,x,y)) {
                CHECK_MESSAGE(forest_score(FR, x, y, w) == 0,
                              "FR not zero outside domain for w=", w.Index,
                              " x=", x, " y=", y);
            }
        }
    };
    expect_zeros_outside(A);
    expect_zeros_outside(B);
    expect_zeros_outside(C);

    // --- 2) Dummy tables: none among A0 in this configuration ---
    for (int y : A0_eps) {
        CHECK(dummy_score(FRDummy, y, A) == 0);
        CHECK(dummy_score(FRDummy, y, B) == 0);
        CHECK(dummy_score(FRDummy, y, C) == 0);
    }

    // --- 3) Expected FR with real-child contributions on endpoints {2,3,4,5,6,7} ---

    // B=[4,7]: no real children; only (x in {5,6,7}, y=7) are in-domain → value 1
    CHECK(forest_score(FR, 5, 7, B) == 1);
    CHECK(forest_score(FR, 6, 7, B) == 1);
    CHECK(forest_score(FR, 7, 7, B) == 1);

    // C=[3,6]: real child is B (4<6<7). Only (x=4, y=7) allows B; else 1.
    CHECK(forest_score(FR, 4, 6, C) == 1);
    CHECK(forest_score(FR, 4, 7, C) == 2); // 1 + FR_B(l_B+1=5, 7)=2
    CHECK(forest_score(FR, 5, 6, C) == 1);
    CHECK(forest_score(FR, 5, 7, C) == 1);
    CHECK(forest_score(FR, 6, 6, C) == 1);
    CHECK(forest_score(FR, 6, 7, C) == 1);

    // A=[2,5]: real children are C and B.
    // x=3: y=5 -> 1; y=6 -> 1 + FR_C(4,6)=2; y=7 -> 1 + max(FR_C(4,7)=2, FR_B(5,7)=1)=3
    CHECK(forest_score(FR, 3, 5, A) == 1);
    CHECK(forest_score(FR, 3, 6, A) == 2);
    CHECK(forest_score(FR, 3, 7, A) == 3);
    // x=4: only B fits when y=7 (x>l_C excludes C); y=5,6 -> 1; y=7 -> 1 + FR_B(5,7)=2
    CHECK(forest_score(FR, 4, 5, A) == 1);
    CHECK(forest_score(FR, 4, 6, A) == 1);
    CHECK(forest_score(FR, 4, 7, A) == 2);
    // x=5: no real child fits → always 1 on domain
    CHECK(forest_score(FR, 5, 5, A) == 1);
    CHECK(forest_score(FR, 5, 6, A) == 1);
    CHECK(forest_score(FR, 5, 7, A) == 1);
}

TEST_CASE("Gavril::computeLeftForestBaseCase: 3 disjoint intervals (all ones in-domain, zeros elsewhere)") {
    using cg::data_structures::Interval;

    // Intervals: [0,1], [2,3], [4,5] — all in A0, pairwise disjoint.
    Interval a(0, 1, 0, 1);
    Interval b(2, 3, 1, 1);
    Interval c(4, 5, 2, 1);
    std::vector<Interval> intervals = {a, b, c};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers and take A0
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(!layers.empty());
    auto& A0 = layers[0];

    // Endpoints for A0 (expected {0,1,2,3,4,5})
    auto EP0 = collect_first_layer_endpoints(A0);
    REQUIRE(EP0.size() == 6);
    CHECK(std::is_sorted(EP0.begin(), EP0.end()));

    // FL table keyed by endpoint values.
    // If m.end is "max endpoint" (e.g., 5), size as m.end+1 instead.
    cg::mif::array4<ForestScore> FL(m.end, kZeroForestScore);
    cg::mif::array4<ChildChoice> FLChoices(m.end, kUnsetChildChoice);

    // Compute base case
    cg::mif::Gavril::computeLeftForestBaseCase(A0, FL, FLChoices);

    auto in_domain = [](const Interval& w, int z, int q){
        // FL base-case domain: z ≤ l_w ≤ q < r_w
        return (z <= w.Left) && (w.Left <= q) && (q < w.Right);
    };

    auto expect_all = [&](const Interval& w){
        for (int z : EP0) for (int q : EP0) {
            const bool dom = in_domain(w, z, q);
            if (dom) {
                // Disjoint ⇒ no real left-children; no left dummies at i=0 ⇒ value must be 1
                CHECK_MESSAGE(forest_score(FL, z, q, w) == 1,
                    "FL should be 1 in-domain for w=", w.Index, " z=", z, " q=", q);
            } else {
                CHECK_MESSAGE(forest_score(FL, z, q, w) == 0,
                    "FL should be 0 outside domain for w=", w.Index, " z=", z, " q=", q);
            }
        }
    };

    expect_all(a); // w = [0,1]
    expect_all(b); // w = [2,3]
    expect_all(c); // w = [4,5]
}


TEST_CASE("Gavril::computeLeftForestBaseCase: real-left transitions exist, only one layer") {
    using cg::data_structures::Interval;

    // Intervals (endpoints 0..5):
    // A=[0,3] (Index=0), B=[2,5] (Index=1), C=[1,4] (Index=2).
    Interval A(0, 3, 0, 1);
    Interval B(2, 5, 1, 1);
    Interval C(1, 4, 2, 1);
    std::vector<Interval> intervals = {A, B, C};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers and take A0
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(!layers.empty());
    auto& A0 = layers[0];

    // (Order doesn't matter for this test, but keep consistent)
    auto EP0 = collect_first_layer_endpoints(A0);
    REQUIRE(EP0.size() == 6);
    CHECK(std::is_sorted(EP0.begin(), EP0.end()));

    // FL table (keyed by endpoint values). If m.end is "max endpoint", use m.end+1.
    cg::mif::array4<ForestScore> FL(m.end, kZeroForestScore);
    cg::mif::array4<ChildChoice> FLChoices(m.end, kUnsetChildChoice);

    // Compute base case
    cg::mif::Gavril::computeLeftForestBaseCase(A0, FL, FLChoices);

    auto in_domain = [](const Interval& w, int z, int q){
        return (z <= w.Left) && (w.Left <= q) && (q < w.Right);
    };

    // 1) Zeros outside i=0 domain
    auto expect_zeros_outside = [&](const Interval& w){
        for (int z : EP0) for (int q : EP0) {
            if (!in_domain(w,z,q)) {
                CHECK_MESSAGE(forest_score(FL, z, q, w) == 0,
                              "FL not zero outside domain for w=", w.Index,
                              " z=", z, " q=", q);
            }
        }
    };
    expect_zeros_outside(A);
    expect_zeros_outside(B);
    expect_zeros_outside(C);

    // 2) Expected FL values (derived manually)

    // A=[0,3]: no real left children. Domain: z=0, q∈{0,1,2} → all 1.
    CHECK(forest_score(FL, 0, 0, A) == 1);
    CHECK(forest_score(FL, 0, 1, A) == 1);
    CHECK(forest_score(FL, 0, 2, A) == 1);

    // C=[1,4]: real left child is A. Needs z<=0 and q>=3 to include A.
    // z=0: q=1→1, q=2→1, q=3→2 (via A with q' = min(3, 3-1=2) => FL_A(0,2)=1)
    CHECK(forest_score(FL, 0, 1, C) == 1);
    CHECK(forest_score(FL, 0, 2, C) == 1);
    CHECK(forest_score(FL, 0, 3, C) == 2);
    // z=1: cannot include A → all 1
    CHECK(forest_score(FL, 1, 1, C) == 1);
    CHECK(forest_score(FL, 1, 2, C) == 1);
    CHECK(forest_score(FL, 1, 3, C) == 1);

    // B=[2,5]: real left children are A and C.
    // z=0: q=2→1; q=3→2 (via A); q=4→3 (via C which gives 2)
    CHECK(forest_score(FL, 0, 2, B) == 1);
    CHECK(forest_score(FL, 0, 3, B) == 2);
    CHECK(forest_score(FL, 0, 4, B) == 3);
    // z=1: q=2→1; q=3→1; q=4→2 (via C only; A excluded by z>0)
    CHECK(forest_score(FL, 1, 2, B) == 1);
    CHECK(forest_score(FL, 1, 3, B) == 1);
    CHECK(forest_score(FL, 1, 4, B) == 2);
    // z=2: no v can satisfy z <= l_v (since l_v ∈ {0,1}) → always 1
    CHECK(forest_score(FL, 2, 2, B) == 1);
    CHECK(forest_score(FL, 2, 3, B) == 1);
    CHECK(forest_score(FL, 2, 4, B) == 1);
}

TEST_CASE("Gavril::computeLeftForestBaseCase: real-left transitions, nested inside two outers") {
    using cg::data_structures::Interval;

    // Inner A0 intervals (endpoints 2..7):
    // A'=[2,5] (Index=0), B'=[4,7] (Index=1), C'=[3,6] (Index=2).
    Interval A(2, 5, 0, 1);
    Interval B(4, 7, 1, 1);
    Interval C(3, 6, 2, 1);

    // Two outer intervals (not in A0): E=[1,8], D=[0,9]
    Interval E(1, 8, 3, 1);
    Interval D(0, 9, 4, 1);

    std::vector<Interval> intervals = {A, B, C, E, D};
    cg::data_structures::DistinctIntervalModel m(intervals);

    // Build layers; A0 should contain exactly A,B,C
    auto layers = cg::interval_model_utils::createLayers(m);
    REQUIRE(!layers.empty());
    auto& A0 = layers[0];

    auto EP0 = collect_first_layer_endpoints(A0);
    REQUIRE(EP0.size() == 6);
    CHECK(std::is_sorted(EP0.begin(), EP0.end()));
    // EP0 should be {2,3,4,5,6,7}

    cg::mif::array4<ForestScore> FL(m.end, kZeroForestScore); // if m.end is max endpoint, use m.end+1
    cg::mif::array4<ChildChoice> FLChoices(m.end, kUnsetChildChoice);

    cg::mif::Gavril::computeLeftForestBaseCase(A0, FL, FLChoices);

    auto in_domain = [](const Interval& w, int z, int q){
        return (z <= w.Left) && (w.Left <= q) && (q < w.Right);
    };

    auto expect_zeros_outside = [&](const Interval& w){
        for (int z : EP0) for (int q : EP0) {
            if (!in_domain(w,z,q)) {
                CHECK_MESSAGE(forest_score(FL, z, q, w) == 0,
                              "FL not zero outside domain for w=", w.Index,
                              " z=", z, " q=", q);
            }
        }
    };
    expect_zeros_outside(A);
    expect_zeros_outside(B);
    expect_zeros_outside(C);

    // A'=[2,5]: no real left children. Domain: z=2, q∈{2,3,4} → all 1.
    CHECK(forest_score(FL, 2, 2, A) == 1);
    CHECK(forest_score(FL, 2, 3, A) == 1);
    CHECK(forest_score(FL, 2, 4, A) == 1);

    // C'=[3,6]: left child is A' only; needs z<=2 and q>=5 → only (z=2,q=5) uses A' → value 2.
    CHECK(forest_score(FL, 2, 3, C) == 1);
    CHECK(forest_score(FL, 2, 4, C) == 1);
    CHECK(forest_score(FL, 2, 5, C) == 2);
    CHECK(forest_score(FL, 3, 3, C) == 1);
    CHECK(forest_score(FL, 3, 4, C) == 1);
    CHECK(forest_score(FL, 3, 5, C) == 1);

    // B'=[4,7]: left children are A' and C'.
    // z=2: q=4→1; q=5→2 (via A'); q=6→3 (via C' which yields 2)
    CHECK(forest_score(FL, 2, 4, B) == 1);
    CHECK(forest_score(FL, 2, 5, B) == 2);
    CHECK(forest_score(FL, 2, 6, B) == 3);
    // z=3: q=4→1; q=5→1; q=6→2 (via C' only)
    CHECK(forest_score(FL, 3, 4, B) == 1);
    CHECK(forest_score(FL, 3, 5, B) == 1);
    CHECK(forest_score(FL, 3, 6, B) == 2);
    // z=4: no v satisfies z <= l_v (l_v∈{2,3}) → always 1
    CHECK(forest_score(FL, 4, 4, B) == 1);
    CHECK(forest_score(FL, 4, 5, B) == 1);
    CHECK(forest_score(FL, 4, 6, B) == 1);
}


