#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

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
    reset();
}

void Fence::insert() {
    MLN_TRACE_FUNC();

    assert(!fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (!fence) {
        auto err = "glFenceSync failed. " + glErrors();
        Log::Error(Event::OpenGL, err);
        throw std::runtime_error(err);
    }
}

bool Fence::isSignaled() const {
    MLN_TRACE_FUNC();

    if (!fence) {
        return false;
    }

    GLenum fenceStatus = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    switch (fenceStatus) {
        case GL_ALREADY_SIGNALED:
            [[fallthrough]];
        case GL_CONDITION_SATISFIED:
            return true;
        case GL_TIMEOUT_EXPIRED:
            return false;
        case GL_WAIT_FAILED: {
            auto err = "glClientWaitSync failed. " + glErrors();
            Log::Error(Event::OpenGL, err);
            throw std::runtime_error(err);
            return false;
        }
        default:
            assert(false); // unreachable
            return false;
    }
}

void Fence::cpuWait() const {
    MLN_TRACE_FUNC();

    if (!fence) {
        constexpr const char* err = "A fence must be inserted in command stream before waiting for it to get signaled";
        Log::Error(Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    constexpr GLuint64 oneSecInNanoSec = 1000000000; // 1 second
    constexpr GLuint64 timeout = oneSecInNanoSec;    // 1 second
    constexpr int retryCount = 10;                   // 10 seconds

    for (int i = 0; i < retryCount; ++i) {
        GLenum fenceStatus = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
        switch (fenceStatus) {
            case GL_ALREADY_SIGNALED:
                [[fallthrough]];
            case GL_CONDITION_SATISFIED:
                return;
            case GL_TIMEOUT_EXPIRED:
                Log::Error(
                    Event::OpenGL,
                    "glClientWaitSync timeout after " + std::to_string(i * timeout / oneSecInNanoSec) + " seconds");
                break;
            case GL_WAIT_FAILED: {
                auto err = "glClientWaitSync failed. " + glErrors();
                Log::Error(Event::OpenGL, err);
                throw std::runtime_error(err);
                return;
            }
            default:
                assert(false); // unreachable
                return;
        }
    }

    throw std::runtime_error("GPU unresponsive after. " + std::to_string(retryCount * timeout / oneSecInNanoSec) +
                             " seconds");
}

void Fence::gpuWait() const {
    MLN_TRACE_FUNC();

    if (!fence) {
        constexpr const char* err = "A fence must be inserted in command stream before waiting for it to get signaled";
        Log::Error(Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    MBGL_CHECK_ERROR(glWaitSync(fence, 0, GL_TIMEOUT_IGNORED));
}

void Fence::reset() noexcept {
    MLN_TRACE_FUNC();

    if (fence) {
        glDeleteSync(fence);
        fence = nullptr;
    }
}

} // namespace gl
} // namespace mbgl
