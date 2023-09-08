#include <mbgl/mtl/buffer_resource.hpp>

#include <Metal/MTLDevice.hpp>

namespace mbgl {
namespace mtl {

BufferResource::BufferResource(MTLDevicePtr device_, const void* data, std::size_t size, NS::UInteger usage_)
    : device(std::move(device_)),
      usage(usage_) {
    if (data && size) {
        buffer = NS::TransferPtr(device->newBuffer(data, static_cast<NS::UInteger>(size), usage));
    } else {
        buffer = NS::TransferPtr(device->newBuffer(static_cast<NS::UInteger>(size), usage));
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

void BufferResource::update(const void* data, std::size_t size, std::size_t offset) {
    if (buffer && data) {
        if (void* content = buffer->contents()) {
            std::memcpy(static_cast<uint8_t*>(content) + offset, data, size);
        }
    }
}

} // namespace mtl
} // namespace mbgl
