#include "mif/maximum_induced_forest_on5.h"

#include "data_structures/interval.h"
#include "utils/array_utils.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace
{
    using Interval = cg::data_structures::Interval;

    class MaximumInducedForestOn5Impl
    {
    public:
        explicit MaximumInducedForestOn5Impl(const cg::data_structures::DistinctIntervalModel &model)
            : _n(model.size),
              _maxEndpoint(0),
              _leftChildren(_n),
              _rightChildren(_n),
              _leftBuilt(static_cast<std::size_t>(_n), false),
              _rightBuilt(static_cast<std::size_t>(_n), false)
        {
            if (_n == 0)
            {
                return;
            }

            auto original = model.getAllIntervals();
            _intervals.reserve(original.size());
            for (const auto &interval : original)
            {
                _intervals.emplace_back(interval.Left + 1, interval.Right + 1, interval.Index, interval.Weight);
            }

            _maxEndpoint = computeMaxEndpoint();
            const int stride = _maxEndpoint + 1;

            _leftForests = cg::utils::array3<int>(_n, stride, stride, 0);
            _rightForests = cg::utils::array3<int>(_n, stride, stride, 0);
            _rootCache = cg::utils::array3<int>(_n, stride, stride, -1);
            _leftDummy = cg::utils::array2<int>(_n, stride, -1);
            _rightDummy = cg::utils::array3<int>(_n, stride, stride, -1);

            _ldInProgress.assign(static_cast<std::size_t>(_n), std::vector<bool>(stride, false));
            _rdInProgress.assign(static_cast<std::size_t>(_n) * stride * stride, false);

            _endpoints.reserve(1 + 2 * static_cast<std::size_t>(_n));
            _endpoints.push_back(0);
            for (const auto &interval : _intervals)
            {
                _endpoints.push_back(interval.Left);
                _endpoints.push_back(interval.Right);
            }
            std::sort(_endpoints.begin(), _endpoints.end());
            _endpoints.erase(std::unique(_endpoints.begin(), _endpoints.end()), _endpoints.end());

            buildChildren();
        }

        [[nodiscard]] int solve()
        {
            if (_n == 0)
            {
                return 0;
            }

            const int Emax = _maxEndpoint;
            int best = 0;
            for (int w = 0; w < _n; ++w)
            {
                const auto &root = _intervals[w];
                for (int split = root.Left; split < root.Right; ++split)
                {
                    const int leftScore = LF_side(w, 0, split);
                    const int rightScore = RF_side(w, split + 1, Emax);
                    best = std::max(best, 1 + leftScore + rightScore);
                }
            }
            return best;
        }

    private:
        std::vector<Interval> _intervals;
        int _n;
        int _maxEndpoint;
        std::vector<int> _endpoints;
        std::vector<std::vector<int>> _leftChildren;
        std::vector<std::vector<int>> _rightChildren;
        cg::utils::array3<int> _leftForests;
        cg::utils::array3<int> _rightForests;
        cg::utils::array3<int> _rootCache;
        cg::utils::array2<int> _leftDummy;
        cg::utils::array3<int> _rightDummy;
        std::vector<bool> _leftBuilt;
        std::vector<bool> _rightBuilt;
        std::vector<std::vector<bool>> _ldInProgress;
        std::vector<bool> _rdInProgress;

        [[nodiscard]] int computeMaxEndpoint() const
        {
            int result = 0;
            for (const auto &interval : _intervals)
            {
                result = std::max(result, interval.Left);
                result = std::max(result, interval.Right);
            }
            return result;
        }

        void buildChildren()
        {
            for (int w = 0; w < _n; ++w)
            {
                const auto &outer = _intervals[w];
                for (int v = 0; v < _n; ++v)
                {
                    if (v == w)
                    {
                        continue;
                    }
                    const auto &inner = _intervals[v];
                    if (inner.Left < outer.Left && outer.Left < inner.Right && inner.Right < outer.Right)
                    {
                        _leftChildren[w].push_back(v);
                    }
                    if (outer.Left < inner.Left && inner.Left < outer.Right && outer.Right < inner.Right)
                    {
                        _rightChildren[w].push_back(v);
                    }
                }
                const auto compareByLeft = [this](int lhs, int rhs)
                {
                    return _intervals[lhs].Left < _intervals[rhs].Left;
                };
                std::sort(_leftChildren[w].begin(), _leftChildren[w].end(), compareByLeft);
                std::sort(_rightChildren[w].begin(), _rightChildren[w].end(), compareByLeft);
            }
        }

        [[nodiscard]] int LF_side(int w, int a, int b)
        {
            if (w < 0 || w >= _n || a < 0 || b < 0 || a > _maxEndpoint || b > _maxEndpoint)
            {
                return 0;
            }
            const auto &interval = _intervals[w];
            if (a == interval.Left)
            {
                if (b > interval.Left && b < interval.Right)
                {
                    return LD(w, b);
                }
                return 0;
            }
            if (!_leftBuilt[static_cast<std::size_t>(w)])
            {
                buildLeft(w);
            }
            return _leftForests(w, a, b);
        }

        [[nodiscard]] int RF_side(int w, int a, int b)
        {
            if (w < 0 || w >= _n || a < 0 || b < 0 || a > _maxEndpoint || b > _maxEndpoint)
            {
                return 0;
            }
            if (!_rightBuilt[static_cast<std::size_t>(w)])
            {
                buildRight(w);
            }
            return _rightForests(w, a, b);
        }

        [[nodiscard]] int ROOT(int v, int L, int R)
        {
            if (v < 0 || v >= _n || L < 0 || R < 0 || L > _maxEndpoint || R > _maxEndpoint)
            {
                return 0;
            }
            int &memo = _rootCache(v, L, R);
            if (memo != -1)
            {
                return memo;
            }

            const auto &interval = _intervals[v];
            if (!(L == interval.Left && interval.Right <= R))
            {
                memo = 0;
                return memo;
            }

            int best = 0;
            for (int split = interval.Left; split < interval.Right; ++split)
            {
                const int leftScore = LF_side(v, L, split);
                const int rightScore = RF_side(v, split + 1, R);
                best = std::max(best, 1 + leftScore + rightScore);
            }
            memo = best;
            return memo;
        }

        [[nodiscard]] int LD(int w, int q)
        {
            const auto &interval = _intervals[w];
            if (q <= interval.Left || q > _maxEndpoint)
            {
                return 0;
            }
            int &memo = _leftDummy(w, q);
            if (memo != -1)
            {
                return memo;
            }
            if (_ldInProgress[static_cast<std::size_t>(w)][q])
            {
                return 0;
            }
            _ldInProgress[static_cast<std::size_t>(w)][q] = true;

            int best = 0;
            for (int v = 0; v < _n; ++v)
            {
                if (v == w)
                {
                    continue;
                }
                const auto &candidate = _intervals[v];
                if (candidate.Left > interval.Left && candidate.Right <= q)
                {
                    best = std::max(best, ROOT(v, candidate.Left, q));
                }
            }

            memo = best;
            _ldInProgress[static_cast<std::size_t>(w)][q] = false;
            return memo;
        }

        [[nodiscard]] bool rdInProgress(int w, int a, int y) const
        {
            const int stride = _maxEndpoint + 1;
            std::size_t index = (static_cast<std::size_t>(w) * stride + static_cast<std::size_t>(a)) * stride + static_cast<std::size_t>(y);
            return _rdInProgress[index];
        }

        void setRdInProgress(int w, int a, int y, bool value)
        {
            const int stride = _maxEndpoint + 1;
            std::size_t index = (static_cast<std::size_t>(w) * stride + static_cast<std::size_t>(a)) * stride + static_cast<std::size_t>(y);
            _rdInProgress[index] = value;
        }

        [[nodiscard]] int RD(int w, int a, int y)
        {
            const auto &interval = _intervals[w];
            if (a < 0 || y < 0 || a > _maxEndpoint || y > _maxEndpoint)
            {
                return 0;
            }
            if (a > y || y <= interval.Right)
            {
                return 0;
            }

            int &memo = _rightDummy(w, a, y);
            if (memo != -1)
            {
                return memo;
            }
            if (rdInProgress(w, a, y))
            {
                return 0;
            }
            setRdInProgress(w, a, y, true);

            int best = 0;
            const int threshold = std::max(a, interval.Right + 1);
            for (int v = 0; v < _n; ++v)
            {
                if (v == w)
                {
                    continue;
                }
                const auto &candidate = _intervals[v];
                if (candidate.Left >= threshold && candidate.Right <= y)
                {
                    best = std::max(best, ROOT(v, candidate.Left, y));
                }
            }

            memo = best;
            setRdInProgress(w, a, y, false);
            return memo;
        }

        void buildLeft(int w)
        {
            if (_leftBuilt[static_cast<std::size_t>(w)])
            {
                return;
            }
            _leftBuilt[static_cast<std::size_t>(w)] = true;

            const auto &root = _intervals[w];
            const int lw = root.Left;
            const int rw = root.Right;

            std::vector<int> Z;
            std::vector<int> Y;
            for (int endpoint : _endpoints)
            {
                if (endpoint < lw)
                {
                    Z.push_back(endpoint);
                }
                else if (endpoint > lw && endpoint < rw)
                {
                    Y.push_back(endpoint);
                }
            }

            if (Z.empty() || Y.empty())
            {
                return;
            }

            const auto &children = _leftChildren[w];
            const int Az = static_cast<int>(Z.size());
            const int By = static_cast<int>(Y.size());
            const int NEG_INF = std::numeric_limits<int>::min() / 4;

            std::vector<int> D(By, 0);
            for (int j = 0; j < By; ++j)
            {
                D[j] = LD(w, Y[j]);
            }

            std::vector<std::vector<int>> S_next(Az, std::vector<int>(By, 0));
            for (int i = 0; i < Az; ++i)
            {
                for (int j = 0; j < By; ++j)
                {
                    S_next[i][j] = D[j];
                }
            }

            for (int childPos = static_cast<int>(children.size()) - 1; childPos >= 0; --childPos)
            {
                const int u = children[childPos];
                const auto &child = _intervals[u];
                const int lu = child.Left;
                const int ru = child.Right;

                std::vector<std::vector<int>> M(Az, std::vector<int>(By, NEG_INF));
                for (int j = 0; j < By; ++j)
                {
                    const int b = Y[j];
                    const int limit = std::min(b, ru);
                    const auto upperIt = std::upper_bound(Y.begin(), Y.end(), limit);
                    const int j_ymax = static_cast<int>(upperIt - Y.begin()) - 1;
                    if (j_ymax < 0)
                    {
                        continue;
                    }

                    std::vector<int> RF_row(j_ymax + 1, 0);
                    for (int jj = 0; jj <= j_ymax; ++jj)
                    {
                        RF_row[jj] = RF_side(u, Y[jj], b);
                    }

                    for (int ii = 0; ii < Az; ++ii)
                    {
                        int best = NEG_INF;
                        for (int jj = 0; jj <= j_ymax; ++jj)
                        {
                            const int cand = S_next[ii][jj] + RF_row[jj];
                            if (cand > best)
                            {
                                best = cand;
                            }
                        }
                        M[ii][j] = best;
                    }
                }

                auto S_cur = S_next;
                for (int i = 0; i < Az; ++i)
                {
                    const int a = Z[i];
                    for (int j = 0; j < By; ++j)
                    {
                        if (D[j] > S_cur[i][j])
                        {
                            S_cur[i][j] = D[j];
                        }

                        const int z_lb = std::max(a, lu);
                        const auto startIt = std::lower_bound(Z.begin(), Z.end(), z_lb);
                        int ii_start = static_cast<int>(startIt - Z.begin());
                        if (ii_start >= Az)
                        {
                            continue;
                        }

                        int best = NEG_INF;
                        for (int ii = ii_start; ii < Az; ++ii)
                        {
                            const int leftValue = LF_side(u, a, Z[ii]);
                            const int bridge = M[ii][j];
                            if (bridge <= NEG_INF / 2)
                            {
                                continue;
                            }
                            const int cand = leftValue + bridge;
                            if (cand > best)
                            {
                                best = cand;
                            }
                        }
                        if (best <= NEG_INF / 2)
                        {
                            continue;
                        }

                        const int cand = 1 + best;
                        if (cand > S_cur[i][j])
                        {
                            S_cur[i][j] = cand;
                        }
                    }
                }

                S_next.swap(S_cur);
            }

            for (int i = 0; i < Az; ++i)
            {
                const int a = Z[i];
                for (int j = 0; j < By; ++j)
                {
                    _leftForests(w, a, Y[j]) = S_next[i][j];
                }
            }
        }

        void buildRight(int w)
        {
            if (_rightBuilt[static_cast<std::size_t>(w)])
            {
                return;
            }
            _rightBuilt[static_cast<std::size_t>(w)] = true;

            const auto &root = _intervals[w];
            const int lw = root.Left;
            const int rw = root.Right;

            std::vector<int> Z;
            std::vector<int> Y;
            for (int endpoint : _endpoints)
            {
                if (endpoint >= lw && endpoint <= rw)
                {
                    Z.push_back(endpoint);
                }
                else if (endpoint > rw)
                {
                    Y.push_back(endpoint);
                }
            }

            if (Z.empty() || Y.empty())
            {
                return;
            }

            const auto &children = _rightChildren[w];
            const int Az = static_cast<int>(Z.size());
            const int By = static_cast<int>(Y.size());
            const int NEG_INF = std::numeric_limits<int>::min() / 4;

            std::vector<std::vector<int>> S_next(Az, std::vector<int>(By, 0));
            for (int i = 0; i < Az; ++i)
            {
                for (int j = 0; j < By; ++j)
                {
                    S_next[i][j] = RD(w, Z[i], Y[j]);
                }
            }

            const auto lowerRwPlusOne = std::lower_bound(Y.begin(), Y.end(), rw + 1);
            const int j_lo_base = static_cast<int>(lowerRwPlusOne - Y.begin());

            for (int childPos = static_cast<int>(children.size()) - 1; childPos >= 0; --childPos)
            {
                const int v = children[childPos];
                const auto &child = _intervals[v];
                const int lv = child.Left;
                const int rv = child.Right;

                std::vector<std::vector<int>> M(Az, std::vector<int>(By, NEG_INF));
                for (int j = 0; j < By; ++j)
                {
                    const int b = Y[j];
                    const int upperBound = std::min(b, rv);
                    const auto hiIt = std::upper_bound(Y.begin(), Y.end(), upperBound);
                    int j_hi = static_cast<int>(hiIt - Y.begin()) - 1;
                    int j_lo = j_lo_base;
                    if (j_lo > j_hi)
                    {
                        continue;
                    }

                    std::vector<int> RF_row;
                    RF_row.reserve(static_cast<std::size_t>(j_hi - j_lo + 1));
                    for (int jx = j_lo; jx <= j_hi; ++jx)
                    {
                        RF_row.push_back(RF_side(v, Y[jx], b));
                    }

                    for (int ii = 0; ii < Az; ++ii)
                    {
                        int best = NEG_INF;
                        for (int offset = 0, jx = j_lo; jx <= j_hi; ++jx, ++offset)
                        {
                            const int cand = S_next[ii][jx] + RF_row[static_cast<std::size_t>(offset)];
                            if (cand > best)
                            {
                                best = cand;
                            }
                        }
                        M[ii][j] = best;
                    }
                }

                auto S_cur = S_next;
                for (int i = 0; i < Az; ++i)
                {
                    const int a = Z[i];
                    const auto startIt = std::lower_bound(Z.begin(), Z.end(), std::max(a, lv));
                    const auto endIt = std::upper_bound(Z.begin(), Z.end(), rw - 1);
                    const int ii_start = static_cast<int>(startIt - Z.begin());
                    const int ii_end = static_cast<int>(endIt - Z.begin()) - 1;
                    if (ii_start > ii_end)
                    {
                        continue;
                    }

                    for (int j = 0; j < By; ++j)
                    {
                        int best = NEG_INF;
                        for (int ii = ii_start; ii <= ii_end; ++ii)
                        {
                            const int bridge = M[ii][j];
                            if (bridge <= NEG_INF / 2)
                            {
                                continue;
                            }
                            const int cand = LF_side(v, a, Z[ii]) + bridge;
                            if (cand > best)
                            {
                                best = cand;
                            }
                        }
                        if (best <= NEG_INF / 2)
                        {
                            continue;
                        }

                        const int cand = 1 + best;
                        if (cand > S_cur[i][j])
                        {
                            S_cur[i][j] = cand;
                        }
                    }
                }

                S_next.swap(S_cur);
            }

            for (int i = 0; i < Az; ++i)
            {
                const int a = Z[i];
                for (int j = 0; j < By; ++j)
                {
                    _rightForests(w, a, Y[j]) = S_next[i][j];
                }
            }
        }
    };
}

int cg::mif::MaximumInducedForestOn5::computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel)
{
    MaximumInducedForestOn5Impl solver(intervalModel);
    return solver.solve();
}

