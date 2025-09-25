#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <webgpu/webgpu.h>

namespace mbgl {
namespace webgpu {

class Context;
class RenderPass;

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(Context& context, const void* data, std::size_t size);
    ~UniformBuffer() override;

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer& operator=(UniformBuffer&&) = delete;

    UniformBuffer clone() const;

    void update(const void* data, std::size_t dataSize) override;

    WGPUBuffer getBuffer() const { return buffer; }

private:
    Context& context;
    WGPUBuffer buffer = nullptr;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArray final : public gfx::UniformBufferArray {
public:
    UniformBufferArray() = default;
    UniformBufferArray(UniformBufferArray&& other)
        : gfx::UniformBufferArray(std::move(other)) {}
    UniformBufferArray(const UniformBufferArray&) = delete;

    UniformBufferArray& operator=(UniformBufferArray&& other) {
        gfx::UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArray& operator=(const UniformBufferArray& other) {
        gfx::UniformBufferArray::operator=(other);
        return *this;
    }

    void bindWebgpu(RenderPass&) const noexcept;
    void bind(gfx::RenderPass& renderPass) override;

private:
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& buffer) override {
        return std::make_unique<UniformBuffer>(static_cast<const UniformBuffer&>(buffer).clone());
    }
};

} // namespace webgpu
} // namespace mbgl
