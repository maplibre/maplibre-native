#pragma once

namespace mln {
namespace util {

namespace non_copyable_ {

class noncopyable {
public:
    noncopyable(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable const&) = delete;

protected:
    constexpr noncopyable() = default;
    ~noncopyable() = default;
};
} // namespace non_copyable_

using non_copyable_::noncopyable;

} // namespace util
} // namespace mln
