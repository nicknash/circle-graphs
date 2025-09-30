#pragma once

#include <ostream>

namespace cg::utils
{
    class LogStream
    {
    public:
        explicit LogStream(bool enabled);

        template <typename T>
        LogStream &operator<<(const T &value)
        {
            if (enabled_)
            {
                stream_ << value;
            }
            return *this;
        }

        LogStream &operator<<(std::ostream &(*manip)(std::ostream &));

    private:
        bool enabled_;
        std::ostream &stream_;
    };
}

