#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/defines.hpp>
#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

Fence::Fence() {}

Fence::~Fence() {
    if (fence) {
        glDeleteSync(fence);
    }
}

void Fence::insert() noexcept {
    assert(!fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool Fence::isSignaled() const noexcept {
    if (!fence) {
        return false;
    }

    return glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
}

} // namespace gl
} // namespace mbgl
