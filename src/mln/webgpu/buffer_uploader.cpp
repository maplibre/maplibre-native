#include <mln/webgpu/buffer_uploader.hpp>
#include <mln/webgpu/renderer_backend.hpp>
#include <mln/util/logging.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace mln {
namespace webgpu {

namespace {
// Copies between buffers need 4-byte aligned offsets and sizes.
constexpr std::uint64_t kCopyAlignment = 4;
// Size of the reused staging buffers. Larger writes get a staging buffer of their own.
constexpr std::uint64_t kStagingBufferSize = 1u << 20;
// Most reused staging buffers to keep, counting those still being mapped.
constexpr std::size_t kMaxPooledBuffers = 8;
// Writes larger than this skip staging and go straight to the queue.
constexpr std::uint64_t kMaxStagedWrite = 64u << 20;

constexpr std::uint64_t alignUp(std::uint64_t value) {
    return (value + kCopyAlignment - 1) & ~(kCopyAlignment - 1);
}
} // namespace

void BufferUploader::onStagingMapped(WGPUMapAsyncStatus status, WGPUStringView, void* userdata, void*) {
    std::unique_ptr<PendingMap> pending(static_cast<PendingMap*>(userdata));
    auto pool = pending->pool.lock();
    if (!pool || pool->released) {
        // The uploader already released this buffer
        return;
    }
    auto& staging = pending->staging;
    std::erase(pool->mapping, staging.buffer);
    if (status == WGPUMapAsyncStatus_Success) {
        staging.mapped = static_cast<std::uint8_t*>(wgpuBufferGetMappedRange(staging.buffer, 0, staging.size));
        if (staging.mapped) {
            staging.used = 0;
            pool->ready.push_back(staging);
            return;
        }
    }
    wgpuBufferRelease(staging.buffer);
}

BufferUploader::BufferUploader(RendererBackend& backend_)
    : backend(backend_),
      pool(std::make_shared<Pool>()) {}

BufferUploader::~BufferUploader() {
    releaseCopies();
    for (auto& staging : active) {
        wgpuBufferRelease(staging.buffer);
    }
    for (auto& staging : pool->ready) {
        wgpuBufferRelease(staging.buffer);
    }
    // A buffer still being mapped may never get its callback once nothing processes events for this
    // device, so release it here. Its callback, if it runs, sees the pool released and does nothing.
    pool->released = true;
    for (auto buffer : std::exchange(pool->mapping, {})) {
        wgpuBufferRelease(buffer);
    }
}

void BufferUploader::write(WGPUBuffer buffer, std::uint64_t offset, const void* data, std::size_t size) {
    if (!buffer || !data || size == 0) {
        return;
    }

    const std::uint64_t bufferSize = wgpuBufferGetSize(buffer);
    const std::uint64_t paddedSize = alignUp(size);
    if (offset % kCopyAlignment != 0 || offset + paddedSize > bufferSize) {
        Log::Error(Event::Render,
                   "WebGPU: buffer write of " + std::to_string(size) + " bytes at offset " + std::to_string(offset) +
                       " doesn't fit a " + std::to_string(bufferSize) + "-byte buffer with 4-byte alignment");
        assert(false);
        return;
    }

    StagingBuffer* staging = paddedSize <= kMaxStagedWrite ? stagingFor(paddedSize) : nullptr;
    if (!staging) {
        // Earlier writes must land first
        submit(nullptr);
        if (WGPUQueue queue = backend.getQueue()) {
            if (paddedSize == size) {
                wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
            } else {
                std::vector<std::uint8_t> padded(paddedSize, 0);
                std::memcpy(padded.data(), data, size);
                wgpuQueueWriteBuffer(queue, buffer, offset, padded.data(), paddedSize);
            }
        }
        return;
    }

    const std::uint64_t sourceOffset = staging->used;
    std::memcpy(staging->mapped + sourceOffset, data, size);
    if (paddedSize > size) {
        std::memset(staging->mapped + sourceOffset + size, 0, paddedSize - size);
    }
    staging->used += paddedSize;

    // Extend the previous copy when this one continues it in both buffers
    if (!copies.empty()) {
        auto& last = copies.back();
        if (last.source == staging->buffer && last.destination == buffer &&
            last.sourceOffset + last.size == sourceOffset && last.destinationOffset + last.size == offset) {
            last.size += paddedSize;
            return;
        }
    }

    wgpuBufferAddRef(buffer);
    copies.push_back({staging->buffer, sourceOffset, buffer, offset, paddedSize});
}

BufferUploader::StagingBuffer* BufferUploader::stagingFor(std::uint64_t size) {
    if (!active.empty() && active.back().size - active.back().used >= size) {
        return &active.back();
    }

    if (size <= kStagingBufferSize) {
        if (pool->ready.empty() && !pool->mapping.empty()) {
            // Collect staging buffers whose mapping has finished
            if (auto instance = static_cast<WGPUInstance>(backend.getInstance())) {
                wgpuInstanceProcessEvents(instance);
            }
        }
        if (!pool->ready.empty()) {
            active.push_back(pool->ready.back());
            pool->ready.pop_back();
            return &active.back();
        }
    }

    WGPUDevice device = backend.getDevice();
    if (!device) {
        return nullptr;
    }
    WGPUBufferDescriptor desc = {};
    desc.label = {"Upload Staging Buffer", WGPU_STRLEN};
    desc.usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc;
    desc.size = std::max(kStagingBufferSize, size);
    desc.mappedAtCreation = 1;

    StagingBuffer staging;
    staging.buffer = wgpuDeviceCreateBuffer(device, &desc);
    if (!staging.buffer) {
        return nullptr;
    }
    staging.size = desc.size;
    staging.mapped = static_cast<std::uint8_t*>(wgpuBufferGetMappedRange(staging.buffer, 0, staging.size));
    if (!staging.mapped) {
        wgpuBufferRelease(staging.buffer);
        return nullptr;
    }
    active.push_back(staging);
    return &active.back();
}

void BufferUploader::submit(WGPUCommandBuffer commands) {
    WGPUQueue queue = backend.getQueue();
    WGPUDevice device = backend.getDevice();

    WGPUCommandBuffer uploadCommands = nullptr;
    if (!copies.empty() && queue && device) {
        for (auto& staging : active) {
            wgpuBufferUnmap(staging.buffer);
            staging.mapped = nullptr;
        }

        WGPUCommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label = {"Buffer Uploads", WGPU_STRLEN};
        if (WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc)) {
            for (const auto& copy : copies) {
                wgpuCommandEncoderCopyBufferToBuffer(
                    encoder, copy.source, copy.sourceOffset, copy.destination, copy.destinationOffset, copy.size);
            }
            uploadCommands = wgpuCommandEncoderFinish(encoder, nullptr);
            wgpuCommandEncoderRelease(encoder);
        }
        if (!uploadCommands) {
            Log::Error(Event::Render, "WebGPU: failed to record buffer uploads");
        }
    }

    if (queue) {
        std::array<WGPUCommandBuffer, 2> buffers{};
        std::size_t count = 0;
        for (auto buffer : {uploadCommands, commands}) {
            if (buffer) {
                buffers[count++] = buffer;
            }
        }
        if (count > 0) {
            wgpuQueueSubmit(queue, count, buffers.data());
        }
    }
    if (uploadCommands) {
        wgpuCommandBufferRelease(uploadCommands);
    }

    for (auto& staging : active) {
        recycle(staging);
    }
    active.clear();
    releaseCopies();
}

void BufferUploader::recycle(StagingBuffer& staging) {
    if (staging.mapped) {
        // Never unmapped, so never used by a submission
        staging.used = 0;
        pool->ready.push_back(staging);
        return;
    }
    if (staging.size != kStagingBufferSize || pool->ready.size() + pool->mapping.size() >= kMaxPooledBuffers) {
        wgpuBufferRelease(staging.buffer);
        return;
    }

    // The callback takes ownership
    auto* pending = std::make_unique<PendingMap>(PendingMap{pool, staging}).release();
    WGPUBufferMapCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    callbackInfo.callback = onStagingMapped;
    callbackInfo.userdata1 = pending;
    pool->mapping.push_back(staging.buffer);
    wgpuBufferMapAsync(staging.buffer, WGPUMapMode_Write, 0, staging.size, callbackInfo);
}

void BufferUploader::releaseCopies() {
    for (const auto& copy : copies) {
        wgpuBufferRelease(copy.destination);
    }
    copies.clear();
}

} // namespace webgpu
} // namespace mln
