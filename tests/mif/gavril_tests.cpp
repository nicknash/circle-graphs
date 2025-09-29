#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "mif/gavril.h"
#include "utils/interval_model_utils.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include <unordered_map>
#include <functional>
#include <set>

#include <iostream>

namespace cgtd = cg::data_structures;

// ---------- Minimal constructor helper (uses your Interval ctor) --------------
static inline cgtd::Interval mk(int L, int R, int idx, int w = 1) {
    if (L > R) std::swap(L, R);
    return cgtd::Interval(L, R, idx, w);
}

// ---------- Endpoint validation ----------------------------------------------
static void validate_endpoints(const std::vector<cgtd::Interval>& ivs) {
    const int n = static_cast<int>(ivs.size());
    REQUIRE(n > 0);
    std::vector<int> eps;
    eps.reserve(2*n);
    for (auto& v : ivs) {
        eps.push_back(v.Left);
        eps.push_back(v.Right);
    }
    std::sort(eps.begin(), eps.end());
    // must be exactly {0,1,...,2n-1}
    for (int i = 0; i < 2*n; ++i) {
        REQUIRE(eps[i] == i);
    }
}

// ---------- Graph helpers ----------------------------------------------------
static bool overlaps_strict(const cgtd::Interval& a, const cgtd::Interval& b) {
    // Edge iff proper intersection without containment.
    return (a.Left < b.Left && b.Left < a.Right && a.Right < b.Right) ||
           (b.Left < a.Left && a.Left < b.Right && b.Right < a.Right);
}

static std::vector<std::vector<int>>
build_adjacency(const std::vector<cgtd::Interval>& ivs) {
    const int n = static_cast<int>(ivs.size());
    std::vector<std::vector<int>> g(n);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (overlaps_strict(ivs[i], ivs[j])) {
                g[i].push_back(j);
                g[j].push_back(i);
            }
    return g;
}

static bool is_forest_induced(const std::vector<cgtd::Interval>& ivs,
                              const std::vector<int>& pick) {
    const auto g_all = build_adjacency(ivs);
    std::unordered_map<int,int> pos_of;
    pos_of.reserve(pick.size());
    for (int p = 0; p < static_cast<int>(pick.size()); ++p) pos_of[pick[p]] = p;

    const int k = static_cast<int>(pick.size());
    std::vector<std::vector<int>> g(k);
    for (int p = 0; p < k; ++p) {
        int v_orig = pick[p];
        for (int w_orig : g_all[v_orig]) {
            auto it = pos_of.find(w_orig);
            if (it != pos_of.end()) g[p].push_back(it->second);
        }
    }

    std::vector<int> color(k, 0);
    std::function<bool(int,int)> dfs = [&](int u, int parent) {
        color[u] = 1;
        for (int v : g[u]) {
            if (v == parent) continue;
            if (color[v] == 1) return false;      // back-edge => cycle
            if (color[v] == 0 && !dfs(v, u)) return false;
        }
        color[u] = 2;
        return true;
    };
    for (int i = 0; i < k; ++i)
        if (color[i] == 0 && !dfs(i, -1)) return false;
    return true;
}

static int brute_force_mif_size(const std::vector<cgtd::Interval>& ivs) {
    const int n = static_cast<int>(ivs.size());
    REQUIRE(n <= 20); // safety; subsets are exponential
    int best = 0;
    std::vector<int> pick;
    pick.reserve(n);
    for (unsigned mask = 1; mask < (1u << n); ++mask) {
        pick.clear();
        for (int i = 0; i < n; ++i)
            if (mask & (1u << i)) pick.push_back(i);
        if (static_cast<int>(pick.size()) <= best) continue;
        if (is_forest_induced(ivs, pick))
            best = static_cast<int>(pick.size());
    }
    return best;
}

// Random permutation -> n intervals pairing (p[2i], p[2i+1]) (distinct endpoints, range 0..2n-1)
static std::vector<cgtd::Interval>
make_intervals_from_permutation(const std::vector<int>& perm) {
    const int n = static_cast<int>(perm.size()) / 2;
    std::vector<cgtd::Interval> ivs;
    ivs.reserve(n);
    for (int i = 0; i < n; ++i) {
        int a = perm[2*i], b = perm[2*i + 1];
        ivs.push_back(mk(a, b, i));
    }
    validate_endpoints(ivs);
    return ivs;
}

static std::vector<int>
gavril_indices(const std::vector<cgtd::Interval>& ivs) {
    const auto chosen = cg::mif::Gavril::computeMif(ivs);
    std::vector<int> out;
    out.reserve(chosen.size());
    for (const auto& iv : chosen) out.push_back(iv.Index);
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

// ---------- Hand-crafted small graphs (now endpoint-valid) -------------------

// n=2, endpoints {0,1,2,3}
TEST_CASE("[Gavril] Single edge is fully kept") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,2,0));
    ivs.push_back(mk(1,3,1));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 2);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

TEST_CASE("[Gavril] Single dummy left child") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,3,0));
    ivs.push_back(mk(1,2,1));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 2);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

TEST_CASE("[Gavril] Three nested intervals") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,5,0));
    ivs.push_back(mk(1,4,1));
    ivs.push_back(mk(2,3,2));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 3);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}


TEST_CASE("[Gavril] Four nested intervals") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,7,0));
    ivs.push_back(mk(1,6,1));
    ivs.push_back(mk(2,5,2));
    ivs.push_back(mk(3,4,3));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 4);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

TEST_CASE("[Gavril] Two disjoint intervals") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,1,0));
    ivs.push_back(mk(2,3,1));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 2);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

TEST_CASE("[Gavril] Three disjoint intervals") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,1,0));
    ivs.push_back(mk(2,3,1));
    ivs.push_back(mk(4,5,2));

    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 3);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}


TEST_CASE("[Gavril] Single interval containing two disjoint") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,5,0));
    ivs.push_back(mk(1,2,1)); 
    ivs.push_back(mk(3,4,2)); 
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 3);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

TEST_CASE("[Gavril] Single interval containing three disjoint") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,7,0));
    ivs.push_back(mk(1,2,1)); 
    ivs.push_back(mk(3,4,2)); 
    ivs.push_back(mk(5,6,3)); 
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 4);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}


// n=3, endpoints {0..5}
TEST_CASE("Gavril: Triangle reduces to 2") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,3,0));
    ivs.push_back(mk(1,4,1));
    ivs.push_back(mk(2,5,2));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 2);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

// n=4, endpoints {0..7}; two disjoint edges: [0,2]&[1,3], [4,6]&[5,7]
TEST_CASE("[Gavril] Disconnected forest stays whole") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,2,0)); // edge 1
    ivs.push_back(mk(1,3,1));
    ivs.push_back(mk(4,6,2)); // edge 2
    ivs.push_back(mk(5,7,3));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 4);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

// n=5, endpoints {0..9}; triangle [0,3],[1,4],[2,5] + disjoint edge [6,8],[7,9]
TEST_CASE("[Gavril] Triangle plus edge") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(0,3,0));
    ivs.push_back(mk(1,4,1));
    ivs.push_back(mk(2,5,2));
    ivs.push_back(mk(6,8,3));
    ivs.push_back(mk(7,9,4));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    REQUIRE(brute == 4);

    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

// n=4, endpoints {0..7}; nested [1,6] ⊃ [2,5] and overlap pair [0,4]–[3,7]
TEST_CASE("[Gavril] Mixed nested + overlaps") {
    std::vector<cgtd::Interval> ivs;
    ivs.push_back(mk(1,6,0)); // nested chain
    ivs.push_back(mk(2,5,1));
    ivs.push_back(mk(0,4,2)); // overlap pair with next
    ivs.push_back(mk(3,7,3));
    validate_endpoints(ivs);

    const int brute = brute_force_mif_size(ivs);
    const auto idxs = gavril_indices(ivs);
    CHECK(static_cast<int>(idxs.size()) == brute);
    CHECK(is_forest_induced(ivs, idxs));
}

// ---------- Randomized vs exhaustive (small n) --------------------------------
TEST_CASE("[Gavril] Random small instances match brute force (n<=9)") {
    std::mt19937 rng(1234567);
    for (int trial = 0; trial < 1; ++trial) {
        int n = 6;//5 + (rng() % 5);           // n in [5..9]
        std::vector<int> perm(2*n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), rng);

        auto ivs = make_intervals_from_permutation(perm);
        for(int i = 0; i < n; ++i)
        {
            std::cout << std::format("{}", ivs[i]) << std::endl;
        }
        for (int i = 0; i < n; ++i) ivs[i].Index = i;
        validate_endpoints(ivs);

        int brute = brute_force_mif_size(ivs);

        std::vector<int> idxs;
        CHECK_NOTHROW( idxs = gavril_indices(ivs) );

        CHECK(static_cast<int>(idxs.size()) == brute);
        CHECK(is_forest_induced(ivs, idxs));
    }
}
