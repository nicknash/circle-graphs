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
    std::vector<Interval> intervals = {
        Interval(0, 7, 0, 1)
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
    // Strict nesting: [1,8] ⊃ [2,7] ⊃ [3,6] ⊃ [4,5]
    std::vector<Interval> intervals = {
        Interval(1, 8, 0, 1),
        Interval(2, 7, 1, 1),
        Interval(3, 6, 2, 1),
        Interval(4, 5, 3, 1),
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
    // Chain A: [0,9] ⊃ [1,8] ⊃ [2,7]
    // Chain B: [10,15] ⊃ [11,14]
    std::vector<Interval> intervals = {
        Interval(0, 9,  0, 1),
        Interval(1, 8,  1, 1),
        Interval(2, 7,  2, 1),
        Interval(10, 15, 3, 1),
        Interval(11, 14, 4, 1),
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Depths by chain: A: 2,1,0 ; B: 1,0
    CHECK(layers.size() == 3);   // max depth = 2
    CHECK(layers[0].size() == 2); // innermost of each chain
    CHECK(layers[1].size() == 2);
    CHECK(layers[2].size() == 1);
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: crossings but only one container")
{
    using cg::data_structures::Interval;
    // [0,9] contains both [1,4] and [5,8]; [2,7] crosses [1,4] and [5,8] (no containment).
    std::vector<Interval> intervals = {
        Interval(0, 9, 0, 1),   // outer
        Interval(2, 7, 1, 1),   // crosses the two inner ones; not a container of them
        Interval(1, 4, 2, 1),   // inner
        Interval(5, 8, 3, 1),   // inner
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Minimal intervals: [1,4], [5,8] → depth 0 (2 of them).
    // [2,7] contains none → depth 0 as well (crossing does not imply containment).
    // [0,9] contains at least one → depth 1.
    CHECK(layers.size() == 2);
    CHECK(layers[0].size() == 3); // three minimals
    CHECK(layers[1].size() == 1); // the unique container
    require_total_count(layers, intervals.size());
}

TEST_CASE("create_layers: complete bipartite containment (bigs contain all smalls)")
{
    using cg::data_structures::Interval;
    // Bigs: B1=[0,11], B2=[1,12]; Smalls: S1=[4,7], S2=[5,8], S3=[6,9]
    std::vector<Interval> intervals = {
        Interval(0, 11, 0, 1), // B1
        Interval(1, 12, 1, 1), // B2
        Interval(4, 7,  2, 1), // S1
        Interval(5, 8,  3, 1), // S2
        Interval(6, 9,  4, 1), // S3
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
    // Random disjoint set
    std::vector<Interval> intervals = {
        Interval(0, 1,  0, 1),
        Interval(3, 4,  1, 1),
        Interval(6, 10, 2, 1),
        Interval(11, 12,3, 1),
        Interval(14, 17,4, 1),
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
    // Chain: [2,15] ⊃ [3,10] ⊃ [4,9]
    // Plus disjoint: [0,1], [16,17]
    std::vector<Interval> intervals = {
        Interval(2, 15, 0, 1),
        Interval(3, 10, 1, 1),
        Interval(4,  9, 2, 1),
        Interval(0,  1, 3, 1),
        Interval(16, 17,4, 1),
    };
    cg::data_structures::DistinctIntervalModel m(intervals);

    auto layers = cg::interval_model_utils::createLayers(m);

    // Depths: chain → 2,1,0; others → 0
    CHECK(layers.size() == 3);
    CHECK(layers[0].size() == 3); // [4,9], [0,1], [16,17]
    CHECK(layers[1].size() == 1); // [3,10]
    CHECK(layers[2].size() == 1); // [2,15]
    require_total_count(layers, intervals.size());
}

