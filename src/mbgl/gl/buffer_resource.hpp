#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/gl/resource_upload_thread_pool.hpp>
#include <mbgl/gl/types.hpp>

#include <condition_variable>
#include <functional>
#include <mutex>

namespace mbgl {
namespace gl {

class BufferResource {
public:
    using AsyncAllocCallback = std::function<UniqueBuffer(int, gfx::BufferUsageType, const void*)>;
    using AsyncUpdateCallback = std::function<void(const UniqueBuffer&, int, const void*)>;

    // Create a vertex buffer resource that takes ownership of buffer_
    BufferResource(UniqueBuffer&& buffer_, int byteSize_);

    // Create a non-owning vertex buffer resource
    BufferResource(AsyncAllocCallback, AsyncUpdateCallback);

    virtual ~BufferResource() noexcept;

    int getByteSize() const { return byteSize; }

    // Access the buffer
    // wait() must be called before pickBuffer() if the buffer is pending an async upload
    const UniqueBuffer& pickBuffer() const;

    // Access the buffer. First call wait() if the buffer is pending an async upload
    const UniqueBuffer& waitAndGetBuffer();

    // Issue an alloc or update command asynchronously to process in the upload pass
    void asyncAlloc(ResourceUploadThreadPool& threadPool,
                    int size,
                    gfx::BufferUsageType usage,
                    const void* initialData = nullptr);
    void asyncUpdate(ResourceUploadThreadPool& threadPool, int size, const void* data);

    // Wait for the async upload to complete
    void wait();

    bool isAsyncPending() const { return asyncUploadRequested; }

private:
    enum class BufferAsyncUploadCommandType : uint8_t {
        Alloc,
        Update,
        None,
    };

    struct BufferAsyncUploadCommand {
        std::vector<uint64_t> data; // only dataSize bytes are valid within data
        int dataSize = 0;
        gfx::BufferUsageType usage;
        BufferAsyncUploadCommandType type = BufferAsyncUploadCommandType::None;
    };

    void issueAsyncUpload();

protected:
    std::unique_ptr<UniqueBuffer> buffer = nullptr;
    int byteSize = 0;

    BufferAsyncUploadCommand asyncUploadCommands;
    AsyncAllocCallback asyncAllocCallback;
    AsyncUpdateCallback asyncUpdateCallback;
    Fence gpuFence;
    bool asyncUploadRequested = false;
    bool asyncUploadIssued = false;
    std::condition_variable asyncUploadConditionVar;
    std::mutex asyncUploadMutex;
};

} // namespace gl
} // namespace mbgl
