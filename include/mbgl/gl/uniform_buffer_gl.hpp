#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/buffer_allocator.hpp>

#include <memory>

namespace mbgl {
namespace gl {

class UniformBufferGL final : public gfx::UniformBuffer {
    UniformBufferGL(const UniformBufferGL&);

public:
    UniformBufferGL(const void* data, std::size_t size_, IBufferAllocator& allocator);
    ~UniformBufferGL() override;

    UniformBufferGL(UniformBufferGL&& rhs) noexcept;
    UniformBufferGL& operator=(const UniformBufferGL& rhs) = delete;

    BufferID getID() const;
    gl::RelocatableBuffer<UniformBufferGL>& getManagedBuffer() noexcept { return managedBuffer; }
    const gl::RelocatableBuffer<UniformBufferGL>& getManagedBuffer() const noexcept { return managedBuffer; }

    UniformBufferGL clone() const { return {*this}; }
    
    const uint8_t* getCurrent() override { return current.get(); };

    // gfx::UniformBuffer
    void update(const void* data, std::size_t size_) override;

private:
    // If the requested UBO size is too large for the allocator, the UBO will manage its own allocation
    bool isManagedAllocation = false;
    BufferID localID;
    gl::RelocatableBuffer<UniformBufferGL> managedBuffer;
    std::unique_ptr<uint8_t[]> current;

    friend class UniformBufferArrayGL;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArrayGL final : public gfx::UniformBufferArray {
public:
    UniformBufferArrayGL() = default;
    UniformBufferArrayGL(UniformBufferArrayGL&& other)
        : UniformBufferArray(std::move(other)) {}
    UniformBufferArrayGL(const UniformBufferArrayGL&) = delete;

    UniformBufferArrayGL& operator=(UniformBufferArrayGL&& other) {
        UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArrayGL& operator=(const UniformBufferArrayGL& other) {
        UniformBufferArray::operator=(other);
        return *this;
    }

private:
    std::unique_ptr<gfx::UniformBuffer> copy(const gfx::UniformBuffer& uniformBuffers) override {
        return std::make_unique<UniformBufferGL>(static_cast<const UniformBufferGL&>(uniformBuffers).clone());
    }
};

} // namespace gl
} // namespace mbgl
