#include <mbgl/gl/context.hpp>
#include <mbgl/gl/buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>
#include <cstring>

namespace mbgl {
namespace gl {

using namespace platform;

BufferResource::BufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::make_unique<UniqueBuffer>(std::move(buffer_))),
      byteSize(byteSize_) {
    assert(buffer);
}

BufferResource::BufferResource(AsyncAllocCallback asyncAllocCallback_, AsyncUpdateCallback asyncUpdateCallback_)
    : asyncAllocCallback(std::move(asyncAllocCallback_)),
      asyncUpdateCallback(std::move(asyncUpdateCallback_)) {
    assert(asyncAllocCallback);
}

BufferResource::~BufferResource() noexcept {
    // We expect the resource to own a buffer at destruction time
    assert(buffer);
    // No pending async upload
    assert(asyncUploadRequested == false);
    assert(asyncUploadCommands.data.empty());
}

const UniqueBuffer& BufferResource::pickBuffer() const {
    // wait must be called first
    assert(asyncUploadRequested == false);
    assert(asyncUploadIssued == false);
    // BufferResource must own a buffer
    assert(buffer);
    return *buffer;
}

const UniqueBuffer& BufferResource::waitAndGetBuffer() {
    MLN_TRACE_FUNC();

    wait();
    return pickBuffer();
}

void BufferResource::asyncAlloc(ResourceUploadThreadPool& threadPool,
                                int size,
                                gfx::BufferUsageType usage,
                                const void* initialData) {
    MLN_TRACE_FUNC();

    assert(asyncAllocCallback);
    // BufferResource must not own a buffer yet since we are allocating one
    assert(!buffer);
    // BufferResource must not have a pending async upload
    assert(asyncUploadIssued == false);
    assert(asyncUploadRequested == false);
    asyncUploadRequested = true;

    auto& cmd = asyncUploadCommands;
    assert(cmd.data.empty());
    assert(cmd.type == BufferAsyncUploadCommandType::None);
    assert(byteSize == 0);
    byteSize = size;
    cmd.type = BufferAsyncUploadCommandType::Alloc;
    cmd.dataSize = size;
    cmd.usage = usage;
    cmd.data.resize((size + sizeof(uint64_t) - 1) / sizeof(uint64_t));
    std::memcpy(cmd.data.data(), initialData, size);

    threadPool.schedule([this] { issueAsyncUpload(); });
}

void BufferResource::asyncUpdate(ResourceUploadThreadPool& threadPool, int size, const void* data) {
    MLN_TRACE_FUNC();

    assert(asyncUpdateCallback);
    // BufferResource must own a buffer to update it
    assert(buffer);
    // BufferResource must not have a pending async upload
    assert(asyncUploadIssued == false);
    assert(asyncUploadRequested == false);
    asyncUploadRequested = true;

    auto& cmd = asyncUploadCommands;
    assert(cmd.data.empty());
    assert(cmd.type == BufferAsyncUploadCommandType::None);
    assert(size <= byteSize);
    byteSize = size;
    cmd.type = BufferAsyncUploadCommandType::Update;
    cmd.dataSize = size;
    cmd.data.resize((size + sizeof(uint64_t) - 1) / sizeof(uint64_t));
    std::memcpy(cmd.data.data(), data, size);

    threadPool.schedule([this] { issueAsyncUpload(); });
}

void BufferResource::wait() {
    MLN_TRACE_FUNC();

    std::unique_lock<std::mutex> lk(asyncUploadMutex);
    if (!asyncUploadRequested) {
        return;
    }
    if (!asyncUploadIssued) {
        asyncUploadConditionVar.wait(lk, [&] { return asyncUploadIssued; });
    }
    assert(asyncUploadCommands.data.empty());
    assert(asyncUploadCommands.type == BufferAsyncUploadCommandType::None);
#ifdef MLN_RENDER_BACKEND_USE_UPLOAD_GL_FENCE
    gpuFence.gpuWait();
    gpuFence.reset();
#endif
    asyncUploadIssued = false;
    asyncUploadRequested = false;
}

void BufferResource::issueAsyncUpload() {
    MLN_TRACE_FUNC();

    const std::lock_guard<std::mutex> lk(asyncUploadMutex);

    assert(asyncUploadRequested == true);
    assert(asyncUploadIssued == false);

    auto& cmd = asyncUploadCommands;
    switch (cmd.type) {
        case BufferAsyncUploadCommandType::Alloc:
            buffer = std::make_unique<UniqueBuffer>(asyncAllocCallback(cmd.dataSize, cmd.usage, cmd.data.data()));
            break;
        case BufferAsyncUploadCommandType::Update:
            asyncUpdateCallback(*buffer, cmd.dataSize, cmd.data.data());
            break;
        default:
            assert(false);
            break;
    }
#ifdef MLN_RENDER_BACKEND_USE_UPLOAD_GL_FENCE
    gpuFence.insert();
#else
    MBGL_CHECK_ERROR(glFlush());
#endif

    cmd.type = BufferAsyncUploadCommandType::None;
    cmd.dataSize = 0;
    cmd.data.clear();
    asyncUploadIssued = true;

    asyncUploadConditionVar.notify_all();
}

} // namespace gl
} // namespace mbgl