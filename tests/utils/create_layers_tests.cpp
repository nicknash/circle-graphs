#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "utils/interval_model_utils.h"

TEST_CASE("create_layers: 3 disjoint intervals all have 0 depth") {
    using cg::data_structures::Interval;

    Interval a(0, 1, 0, 1);
    Interval b(2, 3, 1, 1);
    Interval c(4, 5, 2, 1);

    std::vector<Interval> intervals = {a, b, c};
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    CHECK(layers.size() == 1);
    CHECK(layers[0].size() == 3);
}

// --- helpers (optional) ------------------------------------------------------

static void require_total_count(
    const std::vector<std::vector<cg::data_structures::Interval>>& layers,
    std::size_t expected_total)
{
    std::size_t total = 0;
    for (const auto& L : layers) total += L.size();
    CHECK(total == expected_total);
}

// --- tests -------------------------------------------------------------------

TEST_CASE("create_layers: single interval -> depth 0")
{
    using cg::data_structures::Interval;
    // n=1 → endpoints must be {0,1}
    std::vector<Interval> intervals = {
        Interval(0, 1, 0, 1)
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    CHECK(layers.size() == 1);
    CHECK(layers[0].size() == 1);
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: pure chain of length 4 -> depths 0,1,2,3")
{
    using cg::data_structures::Interval;
    // n=4 → endpoints {0..7}; strict nesting:
    // [0,7] ⊃ [1,6] ⊃ [2,5] ⊃ [3,4]
    std::vector<Interval> intervals = {
        Interval(0, 7, 0, 1), // outermost (depth 3)
        Interval(1, 6, 1, 1),
        Interval(2, 5, 2, 1),
        Interval(3, 4, 3, 1), // innermost (depth 0)
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // layer 0: innermost; layer 3: outermost
    CHECK(layers.size() == 4);
    CHECK(layers[0].size() == 1);
    CHECK(layers[1].size() == 1);
    CHECK(layers[2].size() == 1);
    CHECK(layers[3].size() == 1);
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: two disjoint chains (lengths 3 and 2)")
{
    using cg::data_structures::Interval;
    // n=5 → endpoints {0..9}
    // Chain A on {0..5}: [0,5] ⊃ [1,4] ⊃ [2,3]
    // Chain B on {6..9}: [6,9] ⊃ [7,8]
    std::vector<Interval> intervals = {
        Interval(0, 5, 0, 1), // A3 (depth 2)
        Interval(1, 4, 1, 1), // A2 (depth 1)
        Interval(2, 3, 2, 1), // A1 (depth 0)
        Interval(6, 9, 3, 1), // B2 (depth 1)
        Interval(7, 8, 4, 1), // B1 (depth 0)
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Depths by chain: A: 2,1,0 ; B: 1,0
    CHECK(layers.size() == 3);    // max depth = 2
    CHECK(layers[0].size() == 2); // innermost of each chain
    CHECK(layers[1].size() == 2);
    CHECK(layers[2].size() == 1);
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: crossings but only one container")
{
    using cg::data_structures::Interval;
    // n=4 → endpoints {0..7}
    // One container O=[0,7]; two minimals inside: M1=[1,3], M2=[4,6];
    // Crossing interval X=[2,5] crosses both M1 and M2 but contains none.
    std::vector<Interval> intervals = {
        Interval(0, 7, 0, 1), // outer container → depth 1
        Interval(2, 5, 1, 1), // crosses, no containment → depth 0
        Interval(1, 3, 2, 1), // minimal → depth 0
        Interval(4, 6, 3, 1), // minimal → depth 0
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Minimal intervals: [1,3], [4,6], and [2,5] → depth 0 (3 of them).
    // [0,7] contains at least one → depth 1.
    CHECK(layers.size() == 2);
    CHECK(layers[0].size() == 3); // three minimals
    CHECK(layers[1].size() == 1); // the unique container
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: complete bipartite containment (bigs contain all smalls)")
{
    using cg::data_structures::Interval;
    // n=5 → endpoints {0..9}
    // Bigs cross but neither contains the other: B1=[0,8], B2=[1,9]
    // Smalls all lie in the intersection (1,8): S1=[2,3], S2=[4,5], S3=[6,7]
    std::vector<Interval> intervals = {
        Interval(0, 8, 0, 1), // B1
        Interval(1, 9, 1, 1), // B2
        Interval(2, 3, 2, 1), // S1
        Interval(4, 5, 3, 1), // S2
        Interval(6, 7, 4, 1), // S3
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // All smalls are minimal → depth 0 (3 of them).
    // Bigs contain smalls but no big contains big → depth 1 (2 of them).
    CHECK(layers.size() == 2);
    CHECK(layers[0].size() == 3);
    CHECK(layers[1].size() == 2);
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: disjoint intervals anywhere all have depth 0")
{
    using cg::data_structures::Interval;
    // n=5 → endpoints {0..9}; pair adjacents to ensure disjointness
    std::vector<Interval> intervals = {
        Interval(0, 1,  0, 1),
        Interval(2, 3,  1, 1),
        Interval(4, 5,  2, 1),
        Interval(6, 7,  3, 1),
        Interval(8, 9,  4, 1),
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    CHECK(layers.size() == 1);
    CHECK(layers[0].size() == intervals.size());
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: mixed — one chain plus disjoint others")
{
    using cg::data_structures::Interval;
    // n=5 → endpoints {0..9}
    // Chain: [2,7] ⊃ [3,6] ⊃ [4,5]
    // Disjoint: [0,1], [8,9]
    std::vector<Interval> intervals = {
        Interval(2, 7, 0, 1), // chain outer (depth 2)
        Interval(3, 6, 1, 1), // chain mid   (depth 1)
        Interval(4, 5, 2, 1), // chain inner (depth 0)
        Interval(0, 1, 3, 1), // disjoint    (depth 0)
        Interval(8, 9, 4, 1), // disjoint    (depth 0)
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Depths: chain → 2,1,0; others → 0
    CHECK(layers.size() == 3);
    CHECK(layers[0].size() == 3); // [4,5], [0,1], [8,9]
    CHECK(layers[1].size() == 1); // [3,6]
    CHECK(layers[2].size() == 1); // [2,7]
    require_total_count(layers, intervals.size());
}
