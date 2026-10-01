#pragma once

#include <mln/util/noncopyable.hpp>

namespace mln {

class AsyncRequest : private util::noncopyable {
public:
    virtual ~AsyncRequest() = default;
};

} // namespace mln
