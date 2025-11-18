#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/buffer_allocator.hpp>

namespace mbgl {
namespace gl {

class UniformBufferGL final : public gfx::UniformBuffer {
    UniformBufferGL(const UniformBufferGL&);

public:
    UniformBufferGL(Context& context, const void* data, std::size_t size_, IBufferAllocator& allocator);
    ~UniformBufferGL() override;

    UniformBufferGL(UniformBufferGL&& rhs) noexcept;
    UniformBufferGL& operator=(const UniformBufferGL& rhs) = delete;

    BufferID getID() const;
    gl::RelocatableBuffer<UniformBufferGL>& getManagedBuffer() noexcept { return managedBuffer; }
    const gl::RelocatableBuffer<UniformBufferGL>& getManagedBuffer() const noexcept { return managedBuffer; }

    UniformBufferGL clone() const { return {*this}; }

    // gfx::UniformBuffer
    void update(const void* data, std::size_t dataSize) override;

private:
    Context& context;

    // unique id used for debugging and profiling purposes
    // localID should not be used as unique id because a const buffer pool is managed using IBufferAllocator
    // Currently unique IDs for constant buffers are only used when Tracy profiling is enabled
#ifdef MLN_TRACY_ENABLE
    int64_t uniqueDebugId = -1;
#endif

    // If the requested UBO size is too large for the allocator, the UBO will manage its own allocation
    bool isManagedAllocation = false;
    BufferID localID;
    gl::RelocatableBuffer<UniformBufferGL> managedBuffer;

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

    void bind() const;
    void unbind() const;

    void bind(gfx::RenderPass&) override { bind(); }

private:
    std::unique_ptr<gfx::UniformBuffer> copy(const gfx::UniformBuffer& uniformBuffers) override {
        return std::make_unique<UniformBufferGL>(static_cast<const UniformBufferGL&>(uniformBuffers).clone());
    }
};

} // namespace gl
} // namespace mbgl
