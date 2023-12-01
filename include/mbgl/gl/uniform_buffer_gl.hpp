#pragma once

#include <mbgl/gl/context.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>

namespace mbgl {
namespace gl {

class UniformBufferGL final : public gfx::UniformBuffer {
    UniformBufferGL(const UniformBufferGL&);

public:
    struct Ref {
        UniformBufferGL* refPtr;
        ptrdiff_t bufPtr = 0;
        size_t size = 0;

        Ref(UniformBufferGL* buffer, ptrdiff_t pointer, size_t size_)
            : refPtr(buffer), bufPtr(pointer), size(size_) {}
        Ref(UniformBufferGL* buffer) : refPtr(buffer) {}

        bool operator==(const Ref& rhs) const noexcept {
            return refPtr == rhs.refPtr;
        }
    };

public:
    UniformBufferGL(const void* data, std::size_t size_);
    UniformBufferGL(UniformBufferGL&& other) noexcept
        : UniformBuffer(std::move(other)), contents(std::move(other.contents)) {
        activeBuffer = std::move(other.activeBuffer);
        alignedIndex = std::move(other.alignedIndex);
    }
    ~UniformBufferGL() override;

    BufferID getID() const;
    ptrdiff_t getBaseOffset() const noexcept { return alignedIndex; }
    const std::vector<std::byte>& getContents() const noexcept { return contents; }
    void relocBuffer(size_t bufferID, ptrdiff_t index) noexcept;
    void relocRef(Ref* ref) noexcept;
    void update(const void* data, std::size_t size_) override;

    static void defragment(const gl::Context& context);

    UniformBufferGL clone() const { return {*this}; }

protected:
    Ref* ref = nullptr;
    std::vector<std::byte> contents;
    //uint32_t hash;
    size_t activeBuffer = 0;

    ptrdiff_t alignedIndex = 0;
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
