#pragma once
#include <vector>
#include <limits>
#include <stdexcept>

namespace cg::utils
{
    class FenwickMax
    {
    public:
        explicit FenwickMax(int n = 0, int negInf = std::numeric_limits<int>::min());

        void resize(int n, int negInf = std::numeric_limits<int>::min());

        int size() const;

        // Reset all stored values to negInf (structure size unchanged).
        void reset();

        // Monotone point update: a[idx] = max(a[idx], value), 0 <= idx < N
        void setIdx(int idx, int value);

        // Prefix max over a[0..idx-1]. Accepts idx in [0..N-1]; If N == 0 return neginf
        int prefixMaxInclusive(int idx) const;

    private:
        static int lowbit(int x);

        void require_valid_index(int idx) const;

    private:
        int N;
        int neutral;
        std::vector<int> bit; // 1..N; bit[i] covers (i - lowbit(i) + 1) .. i
    };

}
