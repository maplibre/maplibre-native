#pragma once

#include <webgpu/webgpu.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace mln {
namespace webgpu {

class RendererBackend;

/// Gathers buffer writes and uploads them through reused staging buffers at the next submission.
///
/// `wgpuQueueWriteBuffer` has a high fixed cost per call in some implementations (wgpu-native
/// allocates a staging buffer for each one), and a frame makes hundreds of small writes. Writes
/// queued here keep `wgpuQueueWriteBuffer` ordering: they land in the order they were made, before
/// the command buffer passed to the `submit` that follows them.
///
/// Staging buffers are mapped again after each submission and reused once the mapping completes.
/// Not thread-safe; use it from the render thread only.
class BufferUploader {
public:
    explicit BufferUploader(RendererBackend& backend);
    ~BufferUploader();

    BufferUploader(const BufferUploader&) = delete;
    BufferUploader& operator=(const BufferUploader&) = delete;

    /// Queue a write of `size` bytes to `buffer` at `offset`. The data is copied immediately.
    /// `offset` must be a multiple of 4. `size` is padded to a multiple of 4 when the buffer has room.
    void write(WGPUBuffer buffer, std::uint64_t offset, const void* data, std::size_t size);

    /// Upload the queued writes, then run `commands` (which may be null), in one queue submission.
    void submit(WGPUCommandBuffer commands);

private:
    struct StagingBuffer {
        WGPUBuffer buffer = nullptr;
        std::uint8_t* mapped = nullptr;
        std::uint64_t size = 0;
        std::uint64_t used = 0;
    };

    /// Staging buffers being mapped again, and those ready for reuse. Shared with the map callbacks,
    /// which may run after the uploader is gone.
    struct Pool {
        std::vector<StagingBuffer> ready;
        std::vector<WGPUBuffer> mapping;
        bool released = false;
    };

    struct PendingMap {
        std::weak_ptr<Pool> pool;
        StagingBuffer staging;
    };
    static void onStagingMapped(WGPUMapAsyncStatus, WGPUStringView, void* userdata, void*);

    struct Copy {
        WGPUBuffer source;
        std::uint64_t sourceOffset;
        WGPUBuffer destination;
        std::uint64_t destinationOffset;
        std::uint64_t size;
    };

    StagingBuffer* stagingFor(std::uint64_t size);
    void recycle(StagingBuffer&);
    void releaseCopies();

    RendererBackend& backend;
    std::shared_ptr<Pool> pool;
    std::vector<StagingBuffer> active;
    std::vector<Copy> copies;
};

} // namespace webgpu
} // namespace mln
