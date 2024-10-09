#include <mbgl/gl/resource_upload_thread_pool.hpp>

#include <mbgl/gl/renderer_backend.hpp>

#include <cassert>
#include <thread>

namespace mbgl {
namespace gl {

namespace {

size_t numberOfResourceUploadThreads() {
    size_t hwThreads = std::thread::hardware_concurrency();
    if (hwThreads < 2) {
        return 1;
    }
    // Set the pool size to the number of threads minus one to account for the main render thread
    return std::thread::hardware_concurrency() - 1;
}

std::function<ThreadCallbacks()> makeThreadCallbacks(RendererBackend& backend) {
    assert(backend.supportFreeThreadedUpload());

    // callbacks will run in a separate thread. It is assumed the backend exists during upload

    auto callbackGen = [&]() {
        auto ctx = backend.createUploadThreadContext();
        ThreadCallbacks callbacks;
        callbacks.onThreadBegin = [ctx = ctx]() {
            ctx->createContext();
        };

        callbacks.onThreadEnd = [ctx = ctx]() {
            ctx->destroyContext();
        };

        callbacks.onTaskBegin = [ctx = ctx]() {
            ctx->bindContext();
        };

        callbacks.onTaskEnd = [ctx = ctx]() {
            ctx->unbindContext();
        };
        return callbacks;
    };

    return callbackGen;
}

} // namespace

ResourceUploadThreadPool::ResourceUploadThreadPool(RendererBackend& backend)
    : ThreadedScheduler(numberOfResourceUploadThreads(), false, makeThreadCallbacks(backend), "UploadGL") {}

} // namespace gl
} // namespace mbgl
