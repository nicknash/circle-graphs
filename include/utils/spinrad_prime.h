#pragma once

namespace cg::data_structures
{
    class DistinctIntervalModel;
}

namespace cg::utils
{
    class SpinradPrime
    {
        public:
            bool isPrime(const cg::data_structures::DistinctIntervalModel& intervalModel);
    };
}