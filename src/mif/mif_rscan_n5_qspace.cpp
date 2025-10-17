#include "mif/mif_rscan_n5_qspace.h"

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "utils/array_utils.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace
{
    struct BChoice
    {
        bool has = false;
        int v = -1;
        int s = -1;
    };

    struct SideChoice
    {
        int type = 0; // 0 = none, 1 = switch to B, 2 = attach child
        int child = -1;
        int p = -1;
        int q = -1;
    };

    constexpr int INF_NEG = std::numeric_limits<int>::min() / 4;
}

namespace cg::mif
{
    namespace
    {
        struct ComputationResult
        {
            int size = 0;
            std::vector<cg::data_structures::Interval> picked;
        };

        ComputationResult computeMifInternal(const cg::data_structures::DistinctIntervalModel &intervalModel, bool needSolution)
        {
            const std::vector<cg::data_structures::Interval> intervals = intervalModel.getAllIntervals();
            const int n = static_cast<int>(intervals.size());
            if (n == 0)
            {
                return ComputationResult{};
            }

            const int M = 2 * n;

            cg::utils::array3<int> rightForest(n, M, M, 0);
            cg::utils::array3<int> leftForest(n, M, M, 0);
            cg::utils::array2<int> B(M + 1, M, 0);

            cg::utils::array2<BChoice> bChoices;
            cg::utils::array3<SideChoice> rfChoices;
            cg::utils::array3<SideChoice> lfChoices;
            if (needSolution)
            {
                bChoices = cg::utils::array2<BChoice>(M + 1, M, BChoice{});
                rfChoices = cg::utils::array3<SideChoice>(n, M, M, SideChoice{});
                lfChoices = cg::utils::array3<SideChoice>(n, M, M, SideChoice{});
            }

            const auto getB = [&](int a, int R) -> int
            {
                if (R <= a)
                {
                    return 0;
                }
                return B(a + 1, R);
            };
            const auto setB = [&](int a, int R, int value, const BChoice &choice)
            {
                if (R <= a)
                {
                    return;
                }
                B(a + 1, R) = value;
                if (needSolution)
                {
                    bChoices(a + 1, R) = choice;
                }
            };

            std::vector<int> intervalByRight(M, -1);
            std::vector<int> intervalByLeft(M, -1);
            for (int w = 0; w < n; ++w)
            {
                const auto &interval = intervals[w];
                intervalByRight[interval.Right] = w;
                intervalByLeft[interval.Left] = w;
            }

            for (int R = 0; R < M; ++R)
            {
                for (int L = R; L >= 0; --L)
                {
                    if (L <= R - 1)
                    {
                        int best = 0;
                        BChoice bestChoice;
                        for (int v = 0; v < n; ++v)
                        {
                            const auto &interval = intervals[v];
                            const int lv = interval.Left;
                            const int rv = interval.Right;
                            if (!(L < lv && rv <= R))
                            {
                                continue;
                            }
                            for (int s = lv; s < rv; ++s)
                            {
                                const int left = getB(lv, s);
                                const int right = rightForest(v, s + 1, R);
                                const int candidate = 1 + left + right;
                                if (candidate > best)
                                {
                                    best = candidate;
                                    if (needSolution)
                                    {
                                        bestChoice = BChoice{true, v, s};
                                    }
                                }
                            }
                        }
                        setB(L, R, best, bestChoice);
                    }

                    const int wRight = intervalByRight[L];
                    if (wRight != -1)
                    {
                        const auto &wInterval = intervals[wRight];
                        const int lw = wInterval.Left;
                        const int rw = wInterval.Right;
                        std::vector<std::vector<int>> MR_vp(n, std::vector<int>(M, INF_NEG));
                        std::vector<std::vector<int>> MR_arg_q(n, std::vector<int>(M, -1));
                        for (int v = 0; v < n; ++v)
                        {
                            if (v == wRight)
                            {
                                continue;
                            }
                            const auto &child = intervals[v];
                            const int lv = child.Left;
                            const int rv = child.Right;
                            if (!(rw + 1 <= rv && rv <= R && lv <= rw - 1))
                            {
                                continue;
                            }
                            const int pStart = std::max(lv, lw + 1);
                            for (int p = pStart; p < rw; ++p)
                            {
                                int best_q = INF_NEG;
                                int best_q_arg = -1;
                                for (int q = rw + 1; q <= rv; ++q)
                                {
                                    int inner = 0;
                                    if (p + 1 <= q - 1)
                                    {
                                        inner = rightForest(wRight, p + 1, q - 1);
                                    }
                                    const int val = rightForest(v, q, R);
                                    const int cand_q = val + inner;
                                    if (cand_q > best_q)
                                    {
                                        best_q = cand_q;
                                        best_q_arg = q;
                                    }
                                }
                                MR_vp[v][p] = best_q;
                                MR_arg_q[v][p] = best_q_arg;
                            }
                        }

                        const int base = getB(rw, R);
                        for (int Lp = rw; Lp > lw; --Lp)
                        {
                            int best = base;
                            SideChoice bestChoice;
                            if (needSolution)
                            {
                                bestChoice.type = 1;
                            }
                            for (int v = 0; v < n; ++v)
                            {
                                if (v == wRight)
                                {
                                    continue;
                                }
                                const auto &child = intervals[v];
                                const int lv = child.Left;
                                const int rv = child.Right;
                                if (!(Lp <= lv && lv <= rw - 1 && rw + 1 <= rv && rv <= R))
                                {
                                    continue;
                                }
                                const int pStart = std::max(lv, lw + 1);
                                for (int p = pStart; p < rw; ++p)
                                {
                                    const int mr = MR_vp[v][p];
                                    if (mr == INF_NEG)
                                    {
                                        continue;
                                    }
                                    const int left = leftForest(v, Lp, p);
                                    const int candidate = 1 + left + mr;
                                    if (candidate > best)
                                    {
                                        best = candidate;
                                        if (needSolution)
                                        {
                                            bestChoice.type = 2;
                                            bestChoice.child = v;
                                            bestChoice.p = p;
                                            bestChoice.q = MR_arg_q[v][p];
                                        }
                                    }
                                }
                            }
                            rightForest(wRight, Lp, R) = best;
                            if (needSolution)
                            {
                                rfChoices(wRight, Lp, R) = bestChoice;
                            }
                        }
                    }

                    const int wLeft = intervalByLeft[L];
                    if (wLeft != -1)
                    {
                        const auto &wInterval = intervals[wLeft];
                        const int lw = wInterval.Left;
                        const int rw = wInterval.Right;
                        if (rw > R)
                        {
                            std::vector<std::vector<int>> ML_vp(n, std::vector<int>(M, INF_NEG));
                            std::vector<std::vector<int>> ML_arg_q(n, std::vector<int>(M, -1));
                            for (int v = 0; v < n; ++v)
                            {
                                if (v == wLeft)
                                {
                                    continue;
                                }
                                const auto &child = intervals[v];
                                const int lv = child.Left;
                                const int rv = child.Right;
                                if (!(lv <= lw - 1 && lw + 1 <= rv && rv <= R))
                                {
                                    continue;
                                }
                                for (int p = lv; p < lw; ++p)
                                {
                                    int best_q = INF_NEG;
                                    int best_q_arg = -1;
                                    for (int q = lw + 1; q <= rv; ++q)
                                    {
                                        int inner = 0;
                                        if (p + 1 <= q - 1)
                                        {
                                            inner = leftForest(wLeft, p + 1, q - 1);
                                        }
                                        const int val = rightForest(v, q, R);
                                        const int cand_q = val + inner;
                                        if (cand_q > best_q)
                                        {
                                            best_q = cand_q;
                                            best_q_arg = q;
                                        }
                                    }
                                    ML_vp[v][p] = best_q;
                                    ML_arg_q[v][p] = best_q_arg;
                                }
                            }

                            const int base = getB(lw, R);
                            for (int Lp = lw; Lp >= 0; --Lp)
                            {
                                int best = base;
                                SideChoice bestChoice;
                                if (needSolution)
                                {
                                    bestChoice.type = 1;
                                }
                                for (int v = 0; v < n; ++v)
                                {
                                    if (v == wLeft)
                                    {
                                        continue;
                                    }
                                    const auto &child = intervals[v];
                                    const int lv = child.Left;
                                    const int rv = child.Right;
                                    if (!(Lp <= lv && lv <= lw - 1 && lw + 1 <= rv && rv <= R))
                                    {
                                        continue;
                                    }
                                    for (int p = lv; p < lw; ++p)
                                    {
                                        const int ml = ML_vp[v][p];
                                        if (ml == INF_NEG)
                                        {
                                            continue;
                                        }
                                        const int left = leftForest(v, Lp, p);
                                        const int candidate = 1 + left + ml;
                                        if (candidate > best)
                                        {
                                            best = candidate;
                                            if (needSolution)
                                            {
                                                bestChoice.type = 2;
                                                bestChoice.child = v;
                                                bestChoice.p = p;
                                                bestChoice.q = ML_arg_q[v][p];
                                            }
                                        }
                                    }
                                }
                                leftForest(wLeft, Lp, R) = best;
                                if (needSolution)
                                {
                                    lfChoices(wLeft, Lp, R) = bestChoice;
                                }
                            }
                        }
                    }
                }

                int best = 0;
                BChoice bestChoice;
                for (int v = 0; v < n; ++v)
                {
                    const auto &interval = intervals[v];
                    const int lv = interval.Left;
                    const int rv = interval.Right;
                    if (rv > R)
                    {
                        continue;
                    }
                    for (int s = lv; s < rv; ++s)
                    {
                        const int left = getB(lv, s);
                        const int right = rightForest(v, s + 1, R);
                        const int candidate = 1 + left + right;
                        if (candidate > best)
                        {
                            best = candidate;
                            if (needSolution)
                            {
                                bestChoice = BChoice{true, v, s};
                            }
                        }
                    }
                }
                setB(-1, R, best, bestChoice);
            }

            const int answer = getB(-1, M - 1);
            if (!needSolution)
            {
                return ComputationResult{answer, {}};
            }

            std::vector<char> pickedFlags(n, 0);

            std::function<void(int, int)> backtrackBRec;
            std::function<void(int, int, int)> backtrackRF;
            std::function<void(int, int, int)> backtrackLF;

            backtrackRF = [&](int w, int L, int R)
            {
                if (!(0 <= L && L <= R && R < M))
                {
                    return;
                }
                const auto &wInterval = intervals[w];
                if (!(wInterval.Left < L && L <= wInterval.Right && wInterval.Right <= R))
                {
                    return;
                }
                const SideChoice &choice = rfChoices(w, L, R);
                if (choice.type == 1)
                {
                    backtrackBRec(wInterval.Right, R);
                    return;
                }
                if (choice.type != 2)
                {
                    return;
                }
                const int child = choice.child;
                const int p = choice.p;
                const int q = choice.q;
                if (child < 0)
                {
                    return;
                }
                pickedFlags[child] = 1;
                backtrackLF(child, L, p);
                if (p + 1 <= q - 1)
                {
                    backtrackRF(w, p + 1, q - 1);
                }
                backtrackRF(child, q, R);
            };

            backtrackLF = [&](int w, int L, int R)
            {
                if (!(0 <= L && L <= R && R < M))
                {
                    return;
                }
                const auto &wInterval = intervals[w];
                if (!(L <= wInterval.Left && wInterval.Left <= R && R < wInterval.Right))
                {
                    return;
                }
                const SideChoice &choice = lfChoices(w, L, R);
                if (choice.type == 1)
                {
                    backtrackBRec(wInterval.Left, R);
                    return;
                }
                if (choice.type != 2)
                {
                    return;
                }
                const int child = choice.child;
                const int p = choice.p;
                const int q = choice.q;
                if (child < 0)
                {
                    return;
                }
                pickedFlags[child] = 1;
                backtrackLF(child, L, p);
                if (p + 1 <= q - 1)
                {
                    backtrackLF(w, p + 1, q - 1);
                }
                backtrackRF(child, q, R);
            };

            backtrackBRec = [&](int a, int R)
            {
                if (R <= a)
                {
                    return;
                }
                const BChoice &choice = bChoices(a + 1, R);
                if (!choice.has)
                {
                    return;
                }
                const int v = choice.v;
                const int s = choice.s;
                if (v < 0)
                {
                    return;
                }
                pickedFlags[v] = 1;
                const int leftStart = a + 1;
                if (leftStart >= 0 && leftStart <= s)
                {
                    backtrackLF(v, leftStart, s);
                }
                backtrackRF(v, s + 1, R);
            };

            backtrackBRec(-1, M - 1);

            std::vector<cg::data_structures::Interval> picked;
            picked.reserve(n);
            for (int v = 0; v < n; ++v)
            {
                if (pickedFlags[v])
                {
                    picked.push_back(intervals[v]);
                }
            }
            std::sort(picked.begin(), picked.end(), [](const auto &lhs, const auto &rhs) {
                return lhs.Index < rhs.Index;
            });
            picked.erase(std::unique(picked.begin(), picked.end(), [](const auto &lhs, const auto &rhs) {
                             return lhs.Index == rhs.Index;
                         }),
                         picked.end());
            return ComputationResult{answer, picked};
        }
    }

    int MifRscanN5Qspace::computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        return computeMifInternal(intervalModel, false).size;
    }

    std::pair<int, std::vector<cg::data_structures::Interval>>
    MifRscanN5Qspace::computeMif(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        const auto result = computeMifInternal(intervalModel, true);
        return {result.size, result.picked};
    }
}

