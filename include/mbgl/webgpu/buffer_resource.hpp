#pragma once

#include <mbgl/webgpu/backend_impl.hpp>

#include <cstdint>
#include <cstddef>

namespace mbgl {
namespace webgpu {

class Context;

class BufferResource {
public:
    BufferResource() = default;
    BufferResource(Context& context,
                   const void* data,
                   std::size_t size,
                   uint32_t usage,
                   bool persistent = false);
    ~BufferResource();
    
    Context& getContext() const { return *context; }

    // Disable copy
    BufferResource(const BufferResource&) = delete;
    BufferResource& operator=(const BufferResource&) = delete;

    // Enable move
    BufferResource(BufferResource&& other) noexcept
        : context(other.context),
          buffer(other.buffer),
          size(other.size),
          usage(other.usage),
          persistent(other.persistent) {
        other.buffer = nullptr;
        other.size = 0;
    }

    BufferResource& operator=(BufferResource&& other) noexcept {
        if (this != &other) {
            if (buffer) {
                wgpuBufferRelease(buffer);
            }
            context = other.context;
            buffer = other.buffer;
            size = other.size;
            usage = other.usage;
            persistent = other.persistent;
            other.buffer = nullptr;
            other.size = 0;
        }
        return *this;
    }

    WGPUBuffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }
    uint32_t getUsage() const { return usage; }
    bool isPersistent() const { return persistent; }

    void update(const void* data, std::size_t updateSize, std::size_t offset = 0);

private:
    Context* context = nullptr;
    WGPUBuffer buffer = nullptr;
    std::size_t size = 0;
    uint32_t usage = 0;
    bool persistent = false;
};

} // namespace webgpu
} // namespace mbgl