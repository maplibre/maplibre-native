#pragma once

#include <mbgl/gl/types.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gl {

class Fence {
public:
    Fence();
    ~Fence();

    void insert() noexcept;
    bool isSignaled() const noexcept;

private:
    platform::GLsync fence{nullptr};
};

} // namespace gl
} // namespace mbgl
