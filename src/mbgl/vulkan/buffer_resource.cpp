#include <mbgl/vulkan/buffer_resource.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

#include <algorithm>

namespace mbgl {
namespace vulkan {

BufferResource::BufferResource(Context& context_,
                               const void* data,
                               std::size_t size_,
                               std::uint32_t usage_,
                               bool persistent_)
    : context(context_),
      size(size_),
      usage(usage_),
      persistent(persistent_) {
    


    if (isValid()) {
        auto& stats = context.renderingStats();
        stats.numBuffers++;
        stats.memBuffers += size;
        stats.totalBuffers++;
       
        stats.totalBufferObjs++;
    }
}

BufferResource::BufferResource(BufferResource&& other) noexcept
    : context(other.context),
      raw(std::move(other.raw)),
      size(other.size),
      usage(other.usage),
      persistent(other.persistent) {}

BufferResource::~BufferResource() noexcept {
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
}

BufferResource BufferResource::clone() const {
    return {context, contents(), size, usage, persistent};
}

BufferResource& BufferResource::operator=(BufferResource&& other) noexcept {
    assert(&context == &other.context);
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
    ;
    raw = std::move(other.raw);
    size = other.size;
    usage = other.usage;
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

    std::memcpy(raw.data() + offset, newData, updateSize);
    stats.bufferUpdateBytes += updateSize;

    stats.bufferUpdates++;
    version++;
}

} // namespace mtl
} // namespace mbgl
