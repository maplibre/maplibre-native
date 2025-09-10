#pragma once

#include <webgpu/webgpu_cpp.h>

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
                   wgpu::BufferUsage usage,
                   bool persistent = false);
    ~BufferResource();
    
    Context& getContext() const { return *context; }

    // Disable copy
    BufferResource(const BufferResource&) = delete;
    BufferResource& operator=(const BufferResource&) = delete;

    // Enable move
    BufferResource(BufferResource&& other) noexcept
        : context(other.context),
          buffer(std::move(other.buffer)),
          size(other.size),
          usage(other.usage),
          persistent(other.persistent) {
        other.buffer = nullptr;
        other.size = 0;
    }

    BufferResource& operator=(BufferResource&& other) noexcept {
        if (this != &other) {
            if (buffer) {
                buffer.Destroy();
            }
            context = other.context;
            buffer = std::move(other.buffer);
            size = other.size;
            usage = other.usage;
            persistent = other.persistent;
            other.buffer = nullptr;
            other.size = 0;
        }
        return *this;
    }

    wgpu::Buffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }
    wgpu::BufferUsage getUsage() const { return usage; }
    bool isPersistent() const { return persistent; }

    void update(const void* data, std::size_t updateSize, std::size_t offset = 0);

private:
    Context* context = nullptr;
    wgpu::Buffer buffer;
    std::size_t size = 0;
    wgpu::BufferUsage usage = wgpu::BufferUsage::None;
    bool persistent = false;
};

} // namespace webgpu
} // namespace mbgl