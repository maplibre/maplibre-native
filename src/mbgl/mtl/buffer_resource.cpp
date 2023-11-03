#include <mbgl/mtl/buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderer_backend.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLHeap.hpp>

#include <algorithm>

namespace mbgl {
namespace mtl {

BufferResource::BufferResource(Context& context_, const void* data, std::size_t size_, MTL::ResourceOptions usage_)
    : context(context_),
      size(static_cast<NS::UInteger>(size_)),
      usage(usage_) {
    auto& device = context.getBackend().getDevice();
    auto& heap = context.getHeap();
    // buffer = NS::TransferPtr((data && size) ? device->newBuffer(data, size, usage) : device->newBuffer(size, usage));
    buffer = NS::TransferPtr(heap->newBuffer(static_cast<NS::UInteger>(size), usage));
    if (data && size) {
        update(data, size, 0);
    }
    if (buffer) {
        context.renderingStats().numBuffers++;
        context.renderingStats().memBuffers += size;
    }
}

BufferResource::BufferResource(BufferResource&& other)
    : context(other.context),
      buffer(std::move(other.buffer)),
      size(other.size),
      usage(other.usage) {}

BufferResource::~BufferResource() {
    if (buffer) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
}

BufferResource BufferResource::clone() const {
    return {context, buffer->contents(), size, usage};
}

BufferResource& BufferResource::operator=(BufferResource&& other) {
    assert(&context == &other.context);
    if (buffer) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
    buffer = std::move(other.buffer);
    size = other.size;
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
