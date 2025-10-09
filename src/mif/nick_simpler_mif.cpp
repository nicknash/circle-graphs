#include "mif/nick_simpler_mif.h"

#include "data_structures/interval.h"
#include "utils/array_utils.h"

#include <algorithm>
#include <vector>

namespace
{
    using cg::utils::array3;
    using cg::utils::array4;

    class NickSimplerMifImpl
    {
    public:
        explicit NickSimplerMifImpl(const cg::data_structures::DistinctIntervalModel &intervalModel)
            : _intervals(intervalModel.getAllIntervals()),
              _n(static_cast<int>(_intervals.size())),
              _E(intervalModel.end > 0 ? intervalModel.end - 1 : 0),
              _leftChildren(_n),
              _rightChildren(_n)
        {
            buildChildren();
            initializeTables();
        }

        [[nodiscard]] int run()
        {
            if (_n == 0)
            {
                return 0;
            }
            compute();
            int best = 0;
            for (int w = 0; w < _n; ++w)
            {
                const auto &interval = _intervals[w];
                for (int s = interval.Left; s < interval.Right; ++s)
                {
                    const int leftScore = _LF_full(w, 0, 0, s);
                    const int rightScore = _RF_full(w, 0, s + 1, _E);
                    best = std::max(best, 1 + leftScore + rightScore);
                }
            }
            return best;
        }

    private:
        std::vector<cg::data_structures::Interval> _intervals;
        int _n;
        int _E;
        int _dim = 1;
        int _maxChildCount = 0;
        std::vector<std::vector<int>> _leftChildren;
        std::vector<std::vector<int>> _rightChildren;

        array4<int> _LF_child;
        array4<int> _RF_child;
        array4<int> _LF_full;
        array4<int> _RF_full;
        array3<int> _LD;
        array3<int> _RD;

        void buildChildren()
        {
            _maxChildCount = 0;
            for (int w = 0; w < _n; ++w)
            {
                const auto &intervalW = _intervals[w];
                for (int v = 0; v < _n; ++v)
                {
                    if (v == w)
                    {
                        continue;
                    }
                    const auto &intervalV = _intervals[v];
                    if (intervalV.Left < intervalW.Left && intervalW.Left < intervalV.Right && intervalV.Right < intervalW.Right)
                    {
                        _leftChildren[w].push_back(v);
                    }
                    if (intervalW.Left < intervalV.Left && intervalV.Left < intervalW.Right && intervalW.Right < intervalV.Right)
                    {
                        _rightChildren[w].push_back(v);
                    }
                }
                auto byLeft = [this](int lhs, int rhs)
                {
                    return _intervals[lhs].Left < _intervals[rhs].Left;
                };
                std::sort(_leftChildren[w].begin(), _leftChildren[w].end(), byLeft);
                std::sort(_rightChildren[w].begin(), _rightChildren[w].end(), byLeft);
                _maxChildCount = std::max(_maxChildCount, static_cast<int>(_leftChildren[w].size()));
                _maxChildCount = std::max(_maxChildCount, static_cast<int>(_rightChildren[w].size()));
            }
        }

        void initializeTables()
        {
            _dim = std::max({1, _E + 1, _n + 1, _maxChildCount + 1});
            _LF_child = array4<int>(_dim, 0);
            _RF_child = array4<int>(_dim, 0);
            _LF_full = array4<int>(_dim, 0);
            _RF_full = array4<int>(_dim, 0);
            _LD = array3<int>(_dim, 0);
            _RD = array3<int>(_dim, 0);
        }

        void compute()
        {
            for (int d = 0; d <= _E; ++d)
            {
                phaseOne(d);
                phaseTwo(d);
                phaseThree(d);
            }
        }

        void phaseOne(int d)
        {
            for (int L = 0; L <= _E; ++L)
            {
                const int R = L + d;
                if (R > _E)
                {
                    break;
                }
                for (int w = 0; w < _n; ++w)
                {
                    const auto &intervalW = _intervals[w];
                    computeRFChild(w, L, R, intervalW);
                    computeLFChild(w, L, R, intervalW);
                }
            }
        }

        void computeRFChild(int w, int L, int R, const cg::data_structures::Interval &intervalW)
        {
            const auto &children = _rightChildren[w];
            for (int k = static_cast<int>(children.size()); k >= 0; --k)
            {
                int bestK = 0;
                for (int j = k; j < static_cast<int>(children.size()); ++j)
                {
                    const int v = children[j];
                    const auto &intervalV = _intervals[v];
                    if (!(L <= intervalV.Left && intervalV.Right <= R))
                    {
                        continue;
                    }
                    const int qlo = std::max(L, intervalV.Left);
                    const int qhi = intervalW.Right - 1;
                    const int xlo = intervalW.Right + 1;
                    const int xhi = std::min(R, intervalV.Right);
                    if (qlo > qhi || xlo > xhi)
                    {
                        continue;
                    }
                    for (int qp = qlo; qp <= qhi; ++qp)
                    {
                        for (int xp = xlo; xp <= xhi; ++xp)
                        {
                            const int comp = 1 + _LF_full(v, 0, L, qp) + _RF_full(v, 0, xp, R);
                            const int inner = _RF_child(w, j + 1, qp, xp);
                            bestK = std::max(bestK, comp + inner);
                        }
                    }
                }
                _RF_child(w, k, L, R) = bestK;
            }
        }

        void computeLFChild(int w, int L, int R, const cg::data_structures::Interval &intervalW)
        {
            const auto &children = _leftChildren[w];
            for (int k = static_cast<int>(children.size()); k >= 0; --k)
            {
                int bestK = 0;
                for (int j = k; j < static_cast<int>(children.size()); ++j)
                {
                    const int u = children[j];
                    const auto &intervalU = _intervals[u];
                    if (!(L <= intervalU.Left && intervalU.Right <= R))
                    {
                        continue;
                    }
                    const int zlo = std::max(L, intervalU.Left);
                    const int zhi = intervalW.Left - 1;
                    const int ylo = intervalW.Left + 1;
                    const int yhi = std::min(R, intervalU.Right);
                    if (zlo > zhi || ylo > yhi)
                    {
                        continue;
                    }
                    for (int zp = zlo; zp <= zhi; ++zp)
                    {
                        for (int yp = ylo; yp <= yhi; ++yp)
                        {
                            const int comp = 1 + _LF_full(u, 0, L, zp) + _RF_full(u, 0, yp, R);
                            const int inner = _LF_child(w, j + 1, zp, yp);
                            bestK = std::max(bestK, comp + inner);
                        }
                    }
                }
                _LF_child(w, k, L, R) = bestK;
            }
        }

        void phaseTwo(int d)
        {
            for (int w = 0; w < _n; ++w)
            {
                const auto &intervalW = _intervals[w];
                const int q = intervalW.Left + d;
                if (intervalW.Left + 1 <= q && q <= intervalW.Right - 1)
                {
                    int best = 0;
                    for (int v = 0; v < _n; ++v)
                    {
                        const auto &intervalV = _intervals[v];
                        if (intervalV.Left > intervalW.Left && intervalV.Right <= q)
                        {
                            for (int s = intervalV.Left; s < q; ++s)
                            {
                                const int left = _LF_full(v, 0, intervalV.Left, s);
                                const int right = _RF_full(v, 0, s + 1, q);
                                best = std::max(best, 1 + left + right);
                            }
                        }
                    }
                    _LD(w, q, 0) = best;
                }
            }

            for (int w = 0; w < _n; ++w)
            {
                const auto &intervalW = _intervals[w];
                for (int L = 0; L <= _E; ++L)
                {
                    const int tail = std::max(L, intervalW.Right + 1);
                    const int y = tail + d;
                    if (y < intervalW.Right || y > _E)
                    {
                        continue;
                    }
                    if (tail > y)
                    {
                        _RD(w, L, y) = 0;
                        continue;
                    }
                    int best = 0;
                    for (int v = 0; v < _n; ++v)
                    {
                        const auto &intervalV = _intervals[v];
                        if (intervalV.Left >= tail && intervalV.Right <= y)
                        {
                            for (int s = intervalV.Left; s < y; ++s)
                            {
                                const int left = _LF_full(v, 0, intervalV.Left, s);
                                const int right = _RF_full(v, 0, s + 1, y);
                                best = std::max(best, 1 + left + right);
                            }
                        }
                    }
                    _RD(w, L, y) = best;
                }
            }
        }

        void phaseThree(int d)
        {
            for (int L = 0; L <= _E; ++L)
            {
                const int R = L + d;
                if (R > _E)
                {
                    break;
                }
                for (int w = 0; w < _n; ++w)
                {
                    const auto &intervalW = _intervals[w];
                    computeRFFull(w, L, R, intervalW);
                    computeLFFull(w, L, R, intervalW);
                }
            }
        }

        void computeRFFull(int w, int L, int R, const cg::data_structures::Interval &intervalW)
        {
            const auto &children = _rightChildren[w];
            for (int k = static_cast<int>(children.size()); k >= 0; --k)
            {
                int bestK = 0;
                if (R >= intervalW.Right && std::max(L, intervalW.Right) <= R)
                {
                    int localBest = 0;
                    for (int y = std::max(L, intervalW.Right); y <= R; ++y)
                    {
                        localBest = std::max(localBest, _RD(w, L, y));
                    }
                    bestK = std::max(bestK, localBest);
                }
                for (int j = k; j < static_cast<int>(children.size()); ++j)
                {
                    const int v = children[j];
                    const auto &intervalV = _intervals[v];
                    if (!(L <= intervalV.Left && intervalV.Right <= R))
                    {
                        continue;
                    }
                    const int qlo = std::max(L, intervalV.Left);
                    const int qhi = intervalW.Right - 1;
                    const int xlo = intervalW.Right + 1;
                    const int xhi = std::min(R, intervalV.Right);
                    if (qlo > qhi || xlo > xhi)
                    {
                        continue;
                    }
                    for (int qp = qlo; qp <= qhi; ++qp)
                    {
                        for (int xp = xlo; xp <= xhi; ++xp)
                        {
                            const int comp = 1 + _LF_full(v, 0, L, qp) + _RF_full(v, 0, xp, R);
                            const int inner = _RF_full(w, j + 1, qp, xp);
                            bestK = std::max(bestK, comp + inner);
                        }
                    }
                }
                _RF_full(w, k, L, R) = bestK;
            }
        }

        void computeLFFull(int w, int L, int R, const cg::data_structures::Interval &intervalW)
        {
            const auto &children = _leftChildren[w];
            for (int k = static_cast<int>(children.size()); k >= 0; --k)
            {
                int bestK = 0;
                const int qmax = std::min(R, intervalW.Right - 1);
                if (qmax >= intervalW.Left + 1)
                {
                    int localBest = 0;
                    for (int q = intervalW.Left + 1; q <= qmax; ++q)
                    {
                        localBest = std::max(localBest, _LD(w, q, 0));
                    }
                    bestK = std::max(bestK, localBest);
                }
                for (int j = k; j < static_cast<int>(children.size()); ++j)
                {
                    const int u = children[j];
                    const auto &intervalU = _intervals[u];
                    if (!(L <= intervalU.Left && intervalU.Right <= R))
                    {
                        continue;
                    }
                    const int zlo = std::max(L, intervalU.Left);
                    const int zhi = intervalW.Left - 1;
                    const int ylo = intervalW.Left + 1;
                    const int yhi = std::min(R, intervalU.Right);
                    if (zlo > zhi || ylo > yhi)
                    {
                        continue;
                    }
                    for (int zp = zlo; zp <= zhi; ++zp)
                    {
                        for (int yp = ylo; yp <= yhi; ++yp)
                        {
                            const int comp = 1 + _LF_full(u, 0, L, zp) + _RF_full(u, 0, yp, R);
                            const int inner = _LF_full(w, j + 1, zp, yp);
                            bestK = std::max(bestK, comp + inner);
                        }
                    }
                }
                _LF_full(w, k, L, R) = bestK;
            }
        }
    };
}

namespace cg::mif
{
    int NickSimplerMif::computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        NickSimplerMifImpl impl(intervalModel);
        return impl.run();
    }
}

