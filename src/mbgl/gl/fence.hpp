#pragma once

#include <mbgl/gl/types.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gl {

class Fence {
public:
    Fence();
    ~Fence();

    void insert();
    bool isSignaled() const;
    void cpuWait() const;
    void gpuWait() const;
    void reset() noexcept;

private:
    platform::GLsync fence{nullptr};
};

} // namespace gl
} // namespace mbgl
