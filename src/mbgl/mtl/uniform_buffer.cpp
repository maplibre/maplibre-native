#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

UniformBuffer::UniformBuffer(BufferResource&& buffer_)
    : gfx::UniformBuffer(buffer_.getSizeInBytes()),
      buffer(std::move(buffer_)) {
    buffer.getContext().renderingStats().numUniformBuffers++;
    buffer.getContext().renderingStats().memUniformBuffers += size;
}

UniformBuffer::UniformBuffer(UniformBuffer&& other)
    : gfx::UniformBuffer(std::move(other)),
      buffer(std::move(other.buffer)) {} // NOLINT(bugprone-use-after-move)

UniformBuffer::~UniformBuffer() {
    buffer.getContext().renderingStats().numUniformBuffers--;
    buffer.getContext().renderingStats().memUniformBuffers -= size;
}

void UniformBuffer::update(const void* data, std::size_t dataSize) {
    assert(dataSize <= size);
    if (dataSize > size || dataSize > buffer.getSizeInBytes()) {
        Log::Error(Event::General,
                   "Mismatched size given to UBO update, expected max " + std::to_string(size) + ", got " +
                       std::to_string(dataSize));
        return;
    }

    buffer.getContext().renderingStats().numUniformUpdates++;
    buffer.getContext().renderingStats().uniformUpdateBytes += dataSize;
    buffer.update(data, dataSize, /*offset=*/0);
}

void UniformBufferArray::bindMtl(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < allocatedSize(); id++) {
        const auto& uniformBuffer = get(id);
        if (!uniformBuffer) continue;
        const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer);
        const auto& resource = buffer.getBufferResource();
        if (id != shaders::idDrawableReservedFragmentOnlyUBO) {
            renderPass.bindVertex(resource, 0, id);
        }
        if (id != shaders::idDrawableReservedVertexOnlyUBO) {
            renderPass.bindFragment(resource, 0, id);
        }
    }
}

void UniformBufferArray::bind(gfx::RenderPass& renderPass) {
    bindMtl(static_cast<RenderPass&>(renderPass));
}

} // namespace mtl
} // namespace mbgl
