#include "utils/log_stream.h"

#include <iostream>

namespace cg::utils
{
    LogStream::LogStream(bool enabled)
        : enabled_(enabled), stream_(std::cout)
    {
    }

    LogStream &LogStream::operator<<(std::ostream &(*manip)(std::ostream &))
    {
        if (enabled_)
        {
            stream_ << manip;
        }
        return *this;
    }
}

