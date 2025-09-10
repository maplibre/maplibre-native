#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>

namespace mbgl {
namespace webgpu {

class Context;

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(Context& context, const void* data, std::size_t size);
    ~UniformBuffer() override;

    UniformBuffer(const UniformBuffer&);
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer& operator=(UniformBuffer&&) = delete;

    void update(const void* data, std::size_t dataSize) override;

    WGPUBuffer getBuffer() const { return buffer; }

private:
    Context& context;
    WGPUBuffer buffer = nullptr;
};

class UniformBufferArray final : public gfx::UniformBufferArray {
public:
    UniformBufferArray() = default;
    UniformBufferArray(UniformBufferArray&& other) noexcept
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

    void bind(gfx::RenderPass& renderPass) override;

private:
    std::unique_ptr<gfx::UniformBuffer> copy(const gfx::UniformBuffer& uniformBuffer) override;
};

} // namespace webgpu
} // namespace mbgl