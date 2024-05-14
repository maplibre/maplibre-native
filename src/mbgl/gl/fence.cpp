#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

Fence::Fence() {}

Fence::~Fence() {
    MLN_TRACE_FUNC();

    if (fence) {
        glDeleteSync(fence);
    }
}

void Fence::insert() noexcept {
    MLN_TRACE_FUNC();

    assert(!fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool Fence::isSignaled() const noexcept {
    MLN_TRACE_FUNC();

    if (!fence) {
        return false;
    }

    return glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
}

} // namespace gl
} // namespace mbgl
