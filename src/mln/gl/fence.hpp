#pragma once

#include <mln/gl/types.hpp>
#include <mln/platform/gl_functions.hpp>

namespace mln {
namespace gl {

class Fence {
public:
    Fence();
    ~Fence();

    void insert() noexcept;
    bool isSignaled() const;

private:
    platform::GLsync fence{nullptr};
};

} // namespace gl
} // namespace mln
