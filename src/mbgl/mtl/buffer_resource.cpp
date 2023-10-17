#include <mbgl/mtl/buffer_resource.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLHeap.hpp>
#include <algorithm>

namespace mbgl {
namespace mtl {

BufferResource::BufferResource(MTLDevicePtr device_, const void* data, std::size_t size, MTL::ResourceOptions usage_)
    : device(std::move(device_)),
      usage(usage_) {
    if (data && size) {
        buffer = NS::TransferPtr(device->newBuffer(data, static_cast<NS::UInteger>(size), usage));
    } else {
        buffer = NS::TransferPtr(device->newBuffer(static_cast<NS::UInteger>(size), usage));
    }
}

BufferResource::BufferResource(MTLDevicePtr device_, MTLHeapPtr heap, const void* data, std::size_t size, MTL::ResourceOptions usage_)
    : device(std::move(device_)),
      usage(usage_) {
    buffer = NS::TransferPtr(heap->newBuffer(static_cast<NS::UInteger>(size), usage));
    if (data && size) {
        update(data, size, 0);
    }
}

BufferResource::BufferResource(const BufferResource& other)
    : device(other.device),
      usage(other.usage) {
    if (other.buffer) {
        buffer = NS::TransferPtr(device->newBuffer(other.buffer->contents(), other.buffer->length(), other.usage));
    }
}

BufferResource::BufferResource(BufferResource&& other)
    : device(std::move(other.device)),
      buffer(std::move(other.buffer)),
      usage(other.usage) {}

BufferResource& BufferResource::operator=(BufferResource&& other) {
    device = std::move(other.device);
    buffer = std::move(other.buffer);
    usage = other.usage;
    return *this;
}

void BufferResource::update(const void* data, std::size_t size, std::size_t offset) {
    assert(buffer && (data || size == 0));
    if (buffer && data && size > 0) {
        assert(buffer->contents());
        if (void* content = buffer->contents()) {
            const auto size_ = std::min(size, static_cast<std::size_t>(buffer->length()) - offset);
            assert(size == size_);
            std::memcpy(static_cast<uint8_t*>(content) + offset, data, size_);
        }
    }
}

} // namespace mtl
} // namespace mbgl
