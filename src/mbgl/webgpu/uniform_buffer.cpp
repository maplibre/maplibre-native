#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <webgpu/webgpu.h>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/util/logging.hpp>
#include <cstring>
#include <cstdint>

namespace mbgl {
namespace webgpu {

namespace {
constexpr std::size_t kUniformBufferAlignment = 256u;

constexpr std::size_t alignedUniformSize(std::size_t size) {
    if (size == 0u) {
        return 0u;
    }
    return ((size + (kUniformBufferAlignment - 1u)) / kUniformBufferAlignment) * kUniformBufferAlignment;
}
} // namespace

UniformBuffer::UniformBuffer(Context& context_, const void* data, std::size_t size_)
    : gfx::UniformBuffer(alignedUniformSize(size_)),
      context(context_) {
    const std::size_t alignedSize = getSize();

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (device && alignedSize > 0) {
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView label = {"Uniform Buffer", strlen("Uniform Buffer")};
        bufferDesc.label = label;
        bufferDesc.size = alignedSize;
        bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = data ? 1 : 0;

        buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

        if (buffer && data) {
            void* mappedData = wgpuBufferGetMappedRange(buffer, 0, alignedSize);
            if (mappedData) {
                std::memcpy(mappedData, data, size_);
                if (alignedSize > size_) {
                    std::memset(static_cast<std::uint8_t*>(mappedData) + size_, 0, alignedSize - size_);
                }
                wgpuBufferUnmap(buffer);
            }
        }
    }
}

UniformBuffer UniformBuffer::clone() const {
    UniformBuffer newBuffer(context, nullptr, getSize());

    // Note: We can't directly copy buffer contents in WebGPU without a command encoder
    // The data will need to be copied when updated

    return newBuffer;
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : gfx::UniformBuffer(std::move(other)),
      context(other.context),
      buffer(other.buffer) {
    other.buffer = nullptr;
}

UniformBuffer::~UniformBuffer() {
    if (buffer) {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

void UniformBuffer::update(const void* data, std::size_t dataSize) {
    if (data && dataSize > 0 && buffer) {
        auto& backend = static_cast<RendererBackend&>(context.getBackend());
        WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
        if (queue) {
            wgpuQueueWriteBuffer(queue, buffer, 0, data, dataSize);
        }
    }
}

// UniformBufferArray implementation

void UniformBufferArray::bind(gfx::RenderPass& renderPass) {
    // Note: In WebGPU, uniform buffers are bound through bind groups
    // which are created at the drawable level with the pipeline.
    // This method primarily ensures buffers are ready for use.
    // The actual binding happens in Drawable::draw() via bind groups.
    bindWebgpu(static_cast<RenderPass&>(renderPass));
}

// The copy() method is now implemented inline in the header file to match Metal

void UniformBufferArray::bindWebgpu(RenderPass& renderPass) const noexcept {
    static_cast<void>(renderPass);
    // In WebGPU, uniform buffers are bound through bind groups per drawable,
    // not globally to the render pass like in Metal.
    // The actual binding happens in Drawable::draw when creating the bind group.
    // This method is here for API compatibility with Metal.
}

} // namespace webgpu
} // namespace mbgl
