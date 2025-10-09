#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <cassert>
#include <stdexcept>
#include <string>

namespace mbgl {
namespace gl {

using namespace platform;

namespace {
std::string glErrors() {
    std::string error_list = "";
    for (auto error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        error_list += std::to_string(error) + " ";
    }
    if (error_list.empty()) {
        return error_list;
    }
    return "GL error codes: " + error_list;
}
} // namespace

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
#ifdef __EMSCRIPTEN__
    // WebGL / Emscripten environments may not expose GL sync objects
    // (glFenceSync / glClientWaitSync). As a fallback, flush commands to
    // the driver so work is submitted; we cannot create a GPU fence, so
    // leave `fence` null and emulate completion in `isSignaled()`.
    MBGL_CHECK_ERROR(glFlush());
    fence = nullptr;
#else
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
#endif
}

bool Fence::isSignaled() const {
    MLN_TRACE_FUNC();

    if (!fence) {
        // If we don't have a real GL sync object (e.g. on WASM/WebGL),
        // fall back to a blocking wait to ensure GPU work is complete.
        // This is heavy ( stalls until completion ) but correct. If you
        // need a non-blocking approach, consider emulating fences with
        // timer queries or a different strategy.
#ifdef __EMSCRIPTEN__
        MBGL_CHECK_ERROR(glFinish());
        return true;
#else
        return false;
#endif
    }

    GLenum fenceStatus = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    switch (fenceStatus) {
        case GL_ALREADY_SIGNALED:
            [[fallthrough]];
        case GL_CONDITION_SATISFIED:
            return true;
        case GL_TIMEOUT_EXPIRED:
            return false;
        case GL_WAIT_FAILED:
            throw std::runtime_error("glClientWaitSync failed. " + glErrors());
            return false;
        default:
            assert(false); // unreachable
            return false;
    }
}

} // namespace gl
} // namespace mbgl
