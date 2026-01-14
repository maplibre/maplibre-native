#include <mbgl/mtl/buffer_resource.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderer_backend.hpp>

#include <Metal/MTLDevice.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>

#include <algorithm>

namespace mbgl {
namespace mtl {

namespace {
// The `setVertexBytes:length:atIndex:` method is the best option for binding a very small
// amount (less than 4 KB) of dynamic buffer data to a vertex function, as shown in Listing
// 5-1. This method avoids the overhead of creating an intermediary MTLBuffer object.
// https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/BufferBindings.html
constexpr auto bufferThreshold = 4096;
} // namespace

BufferResource::BufferResource(Context& context_,
                               const void* data,
                               std::size_t size_,
                               MTL::ResourceOptions usage_,
                               bool isIndexBuffer_,
                               bool persistent_)
    : context(context_),
      size(static_cast<NS::UInteger>(size_)),
      usage(usage_),
      isIndexBuffer(isIndexBuffer_),
      persistent(persistent_) {
    // If the buffer is small, not for indexes, and not explicitly persistent, skip the overhead of an `NSBuffer`
    if (size_ < bufferThreshold && !isIndexBuffer_ && !persistent) {
        if (data) {
            raw.assign(static_cast<const std::uint8_t*>(data), static_cast<const std::uint8_t*>(data) + size);
        } else {
            raw.resize(size);
        }
        assert(raw.size() == size);
    } else {
        auto& device = context.getBackend().getDevice();
        buffer = NS::TransferPtr((data && size) ? device->newBuffer(data, size, usage)
                                                : device->newBuffer(size, usage));
    }

    if (isValid()) {
        auto& stats = context.renderingStats();
        stats.numBuffers++;
        stats.memBuffers += size;
        stats.totalBuffers++;
        if (buffer) {
            stats.totalBufferObjs++;
        }
    }
}

BufferResource::BufferResource(BufferResource&& other) noexcept
    : context(other.context),
      buffer(std::move(other.buffer)),
      raw(std::move(other.raw)),
      size(other.size),
      usage(other.usage),
      isIndexBuffer(other.isIndexBuffer),
      persistent(other.persistent) {}

BufferResource::~BufferResource() noexcept {
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
}

BufferResource BufferResource::clone() const {
    return {context, contents(), size, usage, isIndexBuffer, persistent};
}

BufferResource& BufferResource::operator=(BufferResource&& other) noexcept {
    assert(&context == &other.context);
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }

    buffer = std::move(other.buffer);
    raw = std::move(other.raw);
    size = other.size;
    usage = other.usage;
    isIndexBuffer = other.isIndexBuffer;
    persistent = other.persistent;
    return *this;
}

void BufferResource::update(const void* newData, std::size_t updateSize, std::size_t offset) noexcept {
    assert(size >= 0 && updateSize + offset <= size);
    updateSize = std::min(updateSize, size - offset);
    if (updateSize <= 0) {
        return;
    }

    auto& stats = context.renderingStats();
    if (buffer) {
        // Until we can be sure that the buffer is not still in use to render the
        // previous frame, replace it with a new buffer instead of updating it.

        const uint8_t* newBufferSource = nullptr;
        std::unique_ptr<uint8_t[]> tempBuffer;
        const bool updateIsEntireBuffer = (offset == 0 && updateSize == size);

        // If the entire buffer is being updated, make sure it's changed
        if (updateIsEntireBuffer) {
            if (memcmp(buffer->contents(), newData, updateSize) == 0) {
                return;
            }
        }

        auto& device = context.getBackend().getDevice();
        // `[MTLBuffer contents]` may involve memory mapping and/or synchronization.  If the entire
        // buffer is being replaced, avoid accessing the old one by creating the new buffer directly
        // from the given data. If it's just being updated, apply the update to a local buffer to
        // avoid needing to access the new buffer.
        if (updateIsEntireBuffer) {
            newBufferSource = static_cast<const uint8_t*>(newData);
        } else {
            if (auto* const oldContent = static_cast<uint8_t*>(buffer->contents())) {
                tempBuffer.reset(new (std::nothrow) uint8_t[size]);
                assert(tempBuffer);
                if (tempBuffer) {
                    memcpy(tempBuffer.get(), oldContent, size);
                    memcpy(tempBuffer.get() + offset, newData, updateSize);
                    newBufferSource = tempBuffer.get();
                }
            }
        }

        if (newBufferSource) {
            auto newBuffer = NS::TransferPtr(device->newBuffer(newBufferSource, size, usage));
            assert(newBuffer);
            if (newBuffer) {
                buffer = std::move(newBuffer);
                stats.totalBuffers++;
                stats.totalBufferObjs++;
                stats.bufferObjUpdates++;
                stats.bufferUpdateBytes += updateSize;
            }
        }
    } else {
        std::memcpy(raw.data() + offset, newData, updateSize);
        stats.bufferUpdateBytes += updateSize;
    }
    stats.bufferUpdates++;
    version++;
}

bool BufferResource::needReBind(VersionType version_) const noexcept {
    // If we're using a raw buffer, an update means we have to re-bind.
    // For a MTLBuffer, the binding can be left alone.
    return (version != version_);
}

void BufferResource::bindVertex(const MTLRenderCommandEncoderPtr& encoder,
                                const std::size_t offset,
                                const std::size_t index,
                                std::size_t size_) const noexcept {
    assert(offset + size_ <= size);
    if (const auto* mtlBuf = buffer.get()) {
        encoder->setVertexBuffer(mtlBuf, static_cast<NS::UInteger>(offset), static_cast<NS::UInteger>(index));
        usingVertexBuffer = true;
    } else if (!raw.empty()) {
        size_ = size_ ? std::min(size_, size - offset) : size - offset;
        encoder->setVertexBytes(raw.data() + offset, size_, index);
        usingVertexBuffer = false;
    }
}

void BufferResource::bindFragment(const MTLRenderCommandEncoderPtr& encoder,
                                  const std::size_t offset,
                                  const std::size_t index,
                                  std::size_t size_) const noexcept {
    assert(offset + size_ <= size);
    if (const auto* mtlBuf = buffer.get()) {
        encoder->setFragmentBuffer(mtlBuf, static_cast<NS::UInteger>(offset), static_cast<NS::UInteger>(index));
    } else if (!raw.empty()) {
        size_ = size_ ? std::min(size_, size - offset) : size - offset;
        encoder->setFragmentBytes(raw.data() + offset, size_, index);
    }
}

void BufferResource::updateVertexBindOffset(const MTLRenderCommandEncoderPtr& encoder,
                                            std::size_t offset,
                                            std::size_t index,
                                            std::size_t size_) const noexcept {
    // If we're using a MTLBuffer, just update the offset.
    // The documentation for `setVertexBufferOffset` indicates that it should work for buffers
    // assigned using `setVertexBytes` but, in practice, it produces a validation failure:
    // `Set Vertex Buffer Offset Validation index(1) must have an existing buffer.`
    if (buffer.get() && usingVertexBuffer) {
        encoder->setVertexBufferOffset(offset, index);
    } else {
        bindVertex(encoder, offset, index, size_);
    }
}

void BufferResource::updateFragmentBindOffset(const MTLRenderCommandEncoderPtr& encoder,
                                              std::size_t offset,
                                              std::size_t index,
                                              std::size_t size_) const noexcept {
    if (buffer.get()) {
        encoder->setFragmentBufferOffset(offset, index);
    } else {
        bindFragment(encoder, offset, index, size_);
    }
}

} // namespace mtl
} // namespace mbgl
