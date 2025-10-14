#include "mif/nick_simpler_mif.h"

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "utils/array_utils.h"

#include <algorithm>
#include <functional>
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
                B(a + 1, R) = value;
                if (needSolution)
                {
                    bChoices(a + 1, R) = choice;
                }
            };

            for (int width = 0; width <= M; ++width)
            {
                for (int a = -1; a < M - 1; ++a)
                {
                    const int R = a + width;
                    if (R < 0 || R >= M)
                    {
                        continue;
                    }
                    int best = 0;
                    BChoice bestChoice;
                    for (int v = 0; v < n; ++v)
                    {
                        const auto &interval = intervals[v];
                        const int lv = interval.Left;
                        const int rv = interval.Right;
                        if (!(a < lv && rv <= R))
                        {
                            continue;
                        }
                        for (int s = lv; s < rv; ++s)
                        {
                            const int leftStart = a + 1;
                            int leftScore = 0;
                            if (leftStart >= 0 && leftStart <= s)
                            {
                                leftScore = leftForest(v, leftStart, s);
                            }
                            const int candidate = 1 + leftScore + rightForest(v, s + 1, R);
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
                    setB(a, R, best, bestChoice);
                }

                for (int L = 0; L < M; ++L)
                {
                    const int R = L + width;
                    if (R >= M)
                    {
                        break;
                    }
                    for (int w = 0; w < n; ++w)
                    {
                        const auto &wInterval = intervals[w];
                        const int lw = wInterval.Left;
                        const int rw = wInterval.Right;
                        if (!(lw < L && L <= rw && rw <= R))
                        {
                            continue;
                        }
                        int best = getB(rw, R);
                        SideChoice bestChoice;
                        if (needSolution)
                        {
                            bestChoice.type = 1;
                        }
                        for (int v = 0; v < n; ++v)
                        {
                            if (v == w)
                            {
                                continue;
                            }
                            const auto &child = intervals[v];
                            const int lv = child.Left;
                            const int rv = child.Right;
                            if (!(L <= lv && lv <= rw - 1 && rw + 1 <= rv && rv <= R))
                            {
                                continue;
                            }
                            for (int p = lv; p < rw; ++p)
                            {
                                for (int q = rw + 1; q <= rv; ++q)
                                {
                                    const int leftScore = leftForest(v, L, p);
                                    const int rightScore = rightForest(v, q, R);
                                    int middleScore = 0;
                                    if (p + 1 <= q - 1)
                                    {
                                        middleScore = rightForest(w, p + 1, q - 1);
                                    }
                                    const int candidate = 1 + leftScore + rightScore + middleScore;
                                    if (candidate > best)
                                    {
                                        best = candidate;
                                        if (needSolution)
                                        {
                                            bestChoice.type = 2;
                                            bestChoice.child = v;
                                            bestChoice.p = p;
                                            bestChoice.q = q;
                                        }
                                    }
                                }
                            }
                        }
                        rightForest(w, L, R) = best;
                        if (needSolution)
                        {
                            rfChoices(w, L, R) = bestChoice;
                        }
                    }
                }

                for (int L = 0; L < M; ++L)
                {
                    const int R = L + width;
                    if (R >= M)
                    {
                        break;
                    }
                    for (int w = 0; w < n; ++w)
                    {
                        const auto &wInterval = intervals[w];
                        const int lw = wInterval.Left;
                        const int rw = wInterval.Right;
                        if (!(L <= lw && lw <= R && R < rw))
                        {
                            continue;
                        }
                        int best = getB(lw, R);
                        SideChoice bestChoice;
                        if (needSolution)
                        {
                            bestChoice.type = 1;
                        }
                        for (int v = 0; v < n; ++v)
                        {
                            if (v == w)
                            {
                                continue;
                            }
                            const auto &child = intervals[v];
                            const int lv = child.Left;
                            const int rv = child.Right;
                            if (!(L <= lv && lv <= lw - 1 && lw + 1 <= rv && rv <= R))
                            {
                                continue;
                            }
                            for (int p = lv; p < lw; ++p)
                            {
                                for (int q = lw + 1; q <= rv; ++q)
                                {
                                    const int leftScore = leftForest(v, L, p);
                                    const int rightScore = rightForest(v, q, R);
                                    int middleScore = 0;
                                    if (p + 1 <= q - 1)
                                    {
                                        middleScore = leftForest(w, p + 1, q - 1);
                                    }
                                    const int candidate = 1 + leftScore + rightScore + middleScore;
                                    if (candidate > best)
                                    {
                                        best = candidate;
                                        if (needSolution)
                                        {
                                            bestChoice.type = 2;
                                            bestChoice.child = v;
                                            bestChoice.p = p;
                                            bestChoice.q = q;
                                        }
                                    }
                                }
                            }
                        }
                        leftForest(w, L, R) = best;
                        if (needSolution)
                        {
                            lfChoices(w, L, R) = bestChoice;
                        }
                    }
                }
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
                const auto &choice = rfChoices(w, L, R);
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
                const auto &choice = lfChoices(w, L, R);
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
                const auto &choice = bChoices(a + 1, R);
                if (!choice.has)
                {
                    return;
                }
                const int v = choice.v;
                const int s = choice.s;
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

    int NickSimplerMif::computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        return computeMifInternal(intervalModel, false).size;
    }

    std::pair<int, std::vector<cg::data_structures::Interval>>
    NickSimplerMif::computeMif(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        const auto result = computeMifInternal(intervalModel, true);
        return {result.size, result.picked};
    }
}

