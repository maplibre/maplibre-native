#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/mtl/buffer_resource.hpp>

namespace mbgl {
namespace mtl {

class RenderPass;

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(BufferResource&&);
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&&);
    ~UniformBuffer() override;

    const BufferResource& getBufferResource() const { return buffer; }

    UniformBuffer clone() const { return {buffer.clone()}; }

    void update(const void* data, std::size_t dataSize) override;

protected:
    BufferResource buffer;
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

    void bindMtl(RenderPass&) const noexcept;
    void bind(gfx::RenderPass& renderPass) override;

private:
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& buffer) override {
        return std::make_unique<UniformBuffer>(static_cast<const UniformBuffer&>(buffer).clone());
    }
};

} // namespace mtl
} // namespace mbgl
