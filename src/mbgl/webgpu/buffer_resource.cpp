#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

#include <cstring>
#include <cassert>

namespace mbgl {
namespace webgpu {

BufferResource::BufferResource(
    Context& context_, const void* data, std::size_t size_, uint32_t usage_, bool isIndexBuffer_, bool persistent_)
    : context(context_),
      size(size_),
      usage(usage_),
      isIndexBuffer(isIndexBuffer_),
      persistent(persistent_) {
    if (size == 0) {
        return;
    }

    // Store raw data if provided
    if (data) {
        raw.resize(size);
        std::memcpy(raw.data(), data, size);
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) {
        return;
    }

    // Create buffer descriptor
    WGPUBufferDescriptor bufferDesc{};
    WGPUStringView label = {"Buffer Resource", strlen("Buffer Resource")};
    bufferDesc.label = label;
    const std::size_t paddedSize = (size + 3u) & ~std::size_t(3);

    bufferDesc.usage = usage | WGPUBufferUsage_CopyDst;
    bufferDesc.size = paddedSize;
    bufferDesc.mappedAtCreation = 0;

    // Create buffer
    buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

    if (buffer && data) {
        // Upload initial data via queue write
        WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
        if (queue) {
            if (paddedSize == size) {
                wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
            } else {
                std::vector<std::uint8_t> padded(paddedSize, 0);
                std::memcpy(padded.data(), data, size);
                wgpuQueueWriteBuffer(queue, buffer, 0, padded.data(), paddedSize);
            }
        } else {
            // Fallback to mapping when queue is unavailable (should be rare)
            void* mapped = wgpuBufferGetMappedRange(buffer, 0, paddedSize);
            if (mapped) {
                std::memcpy(mapped, data, size);
                if (paddedSize > size) {
                    std::memset(static_cast<std::uint8_t*>(mapped) + size, 0, paddedSize - size);
                }
                wgpuBufferUnmap(buffer);
            }
        }
    }
}

BufferResource::BufferResource(BufferResource&& other) noexcept
    : context(other.context),
      buffer(other.buffer),
      raw(std::move(other.raw)),
      size(other.size),
      usage(other.usage),
      version(other.version),
      isIndexBuffer(other.isIndexBuffer),
      persistent(other.persistent) {
    other.buffer = nullptr;
    other.size = 0;
}

BufferResource::~BufferResource() noexcept {
    if (buffer) {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

BufferResource& BufferResource::operator=(BufferResource&& other) noexcept {
    if (this != &other) {
        // Can't reassign context reference, so check they match
        assert(&context == &other.context);

        if (buffer) {
            wgpuBufferRelease(buffer);
        }
        buffer = other.buffer;
        raw = std::move(other.raw);
        size = other.size;
        usage = other.usage;
        version = other.version;
        isIndexBuffer = other.isIndexBuffer;
        persistent = other.persistent;

        other.buffer = nullptr;
        other.size = 0;
    }
    return *this;
}

BufferResource BufferResource::clone() const {
    return BufferResource(context, raw.empty() ? nullptr : raw.data(), size, usage, isIndexBuffer, persistent);
}

void BufferResource::update(const void* data, std::size_t updateSize, std::size_t offset) noexcept {
    if (!data || updateSize == 0) {
        return;
    }

    // Update raw data if we have it
    if (!raw.empty() && offset + updateSize <= raw.size()) {
        std::memcpy(raw.data() + offset, data, updateSize);
    }

    // Update GPU buffer if we have one
    if (buffer) {
        // Get the queue from backend
        auto& backend = static_cast<RendererBackend&>(context.getBackend());
        WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
        if (queue) {
            // Write data to buffer using queue
            wgpuQueueWriteBuffer(queue, buffer, offset, data, updateSize);
        }
    }

    // Increment version to indicate change
    ++version;
}

bool BufferResource::needReBind(VersionType otherVersion) const noexcept {
    return version != otherVersion;
}

} // namespace webgpu
} // namespace mbgl
