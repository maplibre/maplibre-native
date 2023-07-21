#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/mtl/buffer_resource.hpp>

namespace mbgl {
namespace mtl {

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(BufferResource&&);
    UniformBuffer(const UniformBuffer&) = default;
    UniformBuffer(UniformBuffer&&);
    ~UniformBuffer() override = default;

    const BufferResource& get() const { return buffer; }

    void update(const void* data, std::size_t size_) override;

protected:
    BufferResource buffer;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArray final : public gfx::UniformBufferArray {
public:
    UniformBufferArray(int initCapacity = 10)
        : UniformBufferArray(initCapacity) {}
    UniformBufferArray(UniformBufferArray&& other)
        : UniformBufferArray(std::move(other)) {}
    UniformBufferArray(const UniformBufferArray&) = delete;

    UniformBufferArray& operator=(UniformBufferArray&& other) {
        UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArray& operator=(const UniformBufferArray& other) {
        UniformBufferArray::operator=(other);
        return *this;
    }

private:
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& buffer) override {
        return std::make_unique<UniformBuffer>(static_cast<const UniformBuffer&>(buffer));
    }
};

} // namespace mtl
} // namespace mbgl
