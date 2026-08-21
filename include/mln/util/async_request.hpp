#pragma once

#include <mbgl/util/noncopyable.hpp>

namespace mln {

class AsyncRequest : private util::noncopyable {
public:
    virtual ~AsyncRequest() = default;
};

} // namespace mln
