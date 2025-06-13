#pragma once

namespace cg::data_structures
{
    class DistinctIntervalRep;
}

namespace cg::utils
{
    class SpinradPrime
    {
        public:
            bool isPrime(const cg::data_structures::DistinctIntervalRep& intervalRep);
    };
}