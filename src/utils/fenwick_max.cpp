#include "utils/fenwick_max.h"

namespace cg::utils
{
    FenwickMax::FenwickMax(int n, int neutral)
        : N(0),
          neutral(neutral),
          bit()
    {
        resize(n, neutral);
    }

    void FenwickMax::resize(int n, int negInf)
    {
        if (n < 0)
        {
            throw std::invalid_argument("FenwickMax::resize: n must be non-negative");
        }
        N = n;
        neutral = negInf;
        bit.assign(static_cast<size_t>(N + 1), neutral);
    }

    int FenwickMax::size() const
    {
        return N;
    }

    void FenwickMax::reset()
    {
        for (int i = 0; i <= N; ++i)
        {
            bit[static_cast<size_t>(i)] = neutral;
        }
    }

    int FenwickMax::lowbit(int x)
    {
        return x & -x;
    }

    void FenwickMax::require_valid_index(int idx) const
    {
        if (idx < 0 || idx >= N)
        {
            throw std::out_of_range("FenwickMax: index out of range (expected 0..N-1)");
        }
    }

    void FenwickMax::setIdx(int idx, int value)
    {
        require_valid_index(idx);
        int j = idx + 1; // convert to 1-based indexing.
        for (int i = j; i <= N; i += lowbit(i))
        {
            if (bit[static_cast<size_t>(i)] < value)
            {
                bit[static_cast<size_t>(i)] = value;
            }
        }
    }

    int FenwickMax::prefixMaxInclusive(int idx) const
    {
        if(N == 0)
        {
            return neutral;
        }

        require_valid_index(idx);

        int j = idx + 1; // Convert to one based indexing.
        int res = neutral;
        for (int i = j; i > 0; i -= lowbit(i))
        {
            if (res < bit[static_cast<size_t>(i)])
            {
                res = bit[static_cast<size_t>(i)];
            }
        }
        return res;
    }
}
