#pragma once

#include <mbgl/gl/types.hpp>
#include <type_traits>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <list>
#include <vector>
#include <memory>

namespace mbgl {
namespace gl {

class Context;
class Fence;

class BufferRef {
public:
    BufferRef(size_t pointer, size_t offset, size_t size_)
        : bufferIndex(pointer),
          bufferOffset(offset),
          size(size_) {}

    // Size of our allocation
    size_t getSize() const noexcept { return size; }
    // Index into the array of buffers indicating our current buffer
    size_t getBufferIndex() const noexcept { return bufferIndex; }
    // Offset into the buffer where our allocation begins
    intptr_t getBufferOffset() const noexcept { return bufferOffset; }

private:
    size_t bufferIndex = 0;
    intptr_t bufferOffset = 0;
    size_t size = 0;
};

/// @brief IBufferAllocator hides the underlying implementation of the buffer allocator scheme used.
class IBufferAllocator {
public:
    virtual ~IBufferAllocator(){};

    /// @brief Write data into a buffer managed by the allocator
    /// @param data Pointer to data to write into the buffer
    /// @param size Size in bytes of contents to copy from `data`
    /// @param ref Buffer reference keeping the new allocation alive.
    /// Note: The concrete type of BufferRef is assumed based on the class type of
    /// IBufferAllocator.
    /// @return False on allocation failure.
    virtual bool write(const void* data, size_t size, BufferRef*& ref) noexcept = 0;

    /// @brief Release the allocation held by the given reference
    /// @param ref Allocation reference to release
    virtual void release(BufferRef* ref) noexcept = 0;

    /// Defragment the allocator's underlying buffer pool.
    virtual void defragment(const std::shared_ptr<gl::Fence>& fence) noexcept = 0;

    /// Return the buffer page size used by the allocator. This is the maximum size a
    /// single buffer can possible be.
    virtual size_t pageSize() const noexcept = 0;

    /// Get the OpenGL object ID for the buffer at the given index.
    virtual int32_t getBufferID(size_t bufferIndex) const noexcept = 0;
};

/// @brief A BufferRef holds a strong reference on a buffer sub-allocation, managed by a RelocatableBuffer.
/// @tparam OwnerClass The class type holding a reference on the buffer
template <typename OwnerClass>
class TypedBufferRef : public BufferRef {
public:
    TypedBufferRef() = default;
    TypedBufferRef(OwnerClass* buffer, size_t pointer, size_t offset, size_t size_)
        : BufferRef(pointer, offset, size_),
          ownerPtr(buffer) {}
    TypedBufferRef(OwnerClass* buffer)
        : ownerPtr(buffer) {}

    bool operator==(const TypedBufferRef& rhs) const noexcept { return ownerPtr == rhs.ownerPtr; }

    // The owner of this allocation
    OwnerClass* getOwner() const noexcept { return ownerPtr; }
    void setOwner(OwnerClass* owner) { ownerPtr = owner; }

private:
    OwnerClass* ownerPtr = nullptr;
};

/// A RelocatableBuffer is in essence a view onto an actual buffer. These actual buffers may move around
/// over time as fragmentation is managed and buffers are recycled. A RelocatableBuffer is aware of this
/// and actively participates in this memory relocation.
template <typename OwnerClass>
class RelocatableBuffer {
public:
    RelocatableBuffer(IBufferAllocator& allocator_, OwnerClass* owner_)
        : allocator(allocator_),
          owner(owner_) {
        assert(owner);
    }
    RelocatableBuffer(const RelocatableBuffer<OwnerClass>& rhs)
        : allocator(rhs.allocator),
          owner(rhs.owner),
          contents(rhs.contents) {
        allocate(contents.data(), contents.size());
    }
    RelocatableBuffer(RelocatableBuffer<OwnerClass>&& rhs) noexcept
        : allocator(rhs.allocator),
          owner(rhs.owner),
          ref(std::move(rhs.ref)),
          contents(std::move(rhs.contents)) {}
    ~RelocatableBuffer() {
        if (ref) {
            assert(ref->getOwner());
            allocator.release(ref);
        }
    }

    // As references are added to a buffer, reallocation of the underlying reference
    // storage may occur. When that happens, we will be informed of the new memory location
    // of our reference here.
    void relocRef(TypedBufferRef<OwnerClass>* newRef) noexcept { ref = newRef; }

    // Get the current OpenGL buffer ID. Do not store this, bind it and discard after the active frame.
    int32_t getBufferID() const noexcept { return ref ? allocator.getBufferID(ref->getBufferIndex()) : 0; }

    intptr_t getBindingOffset() const noexcept { return ref ? ref->getBufferOffset() : 0; }

    const std::vector<std::byte>& getContents() const noexcept { return contents; }

    void setOwner(OwnerClass* owner_) noexcept { owner = owner_; }

    /// Allocate buffer memory and copy `size` bytes into the allocation from `data`
    void allocate(const void* data, size_t size) noexcept {
        assert(owner);

        if (ref && ref->getOwner()) {
            // If we're writing new data, we need to remove our ref from our old buffer.
            allocator.release(ref);
            ref = nullptr;
        }

        BufferRef* reference = ref;
        allocator.write(data, size, reference);

        ref = std::move(static_cast<decltype(ref)>(reference));
        ref->setOwner(owner);

        contents.resize(size);
        std::memcpy(contents.data(), data, size);
    };

    IBufferAllocator& allocator;

private:
    OwnerClass* owner = nullptr;
    TypedBufferRef<OwnerClass>* ref = nullptr; // A strong reference to the active allocation backing this buffer
    std::vector<std::byte> contents;           // CPU-side buffer contents
};

class UniformBufferAllocator : public IBufferAllocator {
public:
    UniformBufferAllocator();
    ~UniformBufferAllocator() override;

    bool write(const void* data, size_t size, BufferRef*& ref) noexcept override;
    void release(BufferRef* ref) noexcept override;
    void defragment(const std::shared_ptr<gl::Fence>& fence) noexcept override;
    size_t pageSize() const noexcept override;
    int32_t getBufferID(size_t bufferIndex) const noexcept override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gl
} // namespace mbgl
