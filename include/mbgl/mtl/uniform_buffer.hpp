#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>

namespace mbgl {
namespace mtl {

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(const void* data, std::size_t size_);
    UniformBuffer(const UniformBuffer& other)
        : UniformBuffer(other) {}
    UniformBuffer(UniformBuffer&& other)
        : UniformBuffer(std::move(other)) {}
    ~UniformBuffer() override;

    //BufferID getID() const { return id; }
    void update(const void* data, std::size_t size_) override;

protected:
    //BufferID id = 0;
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
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& uniformBuffers) override {
        return gfx::UniqueUniformBuffer(
            new UniformBuffer(static_cast<const UniformBuffer&>(uniformBuffers)));
    }
};

} // namespace mtl
} // namespace mbgl
