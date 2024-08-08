#include <mbgl/gl/buffer_allocator.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <utility>

namespace mbgl {

using namespace platform;

namespace gl {

/// @brief BufferAllocator is a semi-generic allocation strategy for uniform buffer objects.
/// This allocator works by streaming buffer writes into sub-allocated sections of actual UBOs.
/// As UBOs fill up, they are marked 'in-flight' and placed in a waiting area for references to
/// be released. Once all references to a UBO are gone (Or the buffer is defragmented), the
/// buffer is recycled.
/// @tparam OwnerClass The class type having allocations managed by this allocator.
/// @tparam type GL_UNIFORM_BUFFER, no other type is currently valid.
/// @tparam PageSizeKB Size of underlying UBO allocations, in KB. 8KB has been observed to work well.
/// @tparam MaxFragmentationOccupancy Buffers under this percentage of occupancy are considered fragmented
/// and the defragmentation routine will attempt to relocate living references to them to other buffers.
/// @tparam MaxFreeBuffers When buffers are made free for recycling, buffers in excess of this amount are
/// released to reduce memory consumption.
/// @tparam InitialBufferSize Initial size of the reference list. 128 has been observed to work well.
template <typename OwnerClass, // UniformBufferGL
          GLenum type,         // GL_UNIFORM_BUFFER
          size_t PageSizeKB = 8,
          size_t MaxFragmentationOccupancy = 10,
          size_t MaxFreeBuffers = 48,
          size_t InitialBufferSize = 256>
class BufferAllocator : public IBufferAllocator {
    static_assert(type == GL_UNIFORM_BUFFER);

public:
    static constexpr const size_t PageSize = 1024 * PageSizeKB;
    static constexpr const size_t FragmentedOccupancyLevel = static_cast<size_t>(
        PageSize * (static_cast<double>(MaxFragmentationOccupancy) / 100.0));
    using BufferTy =
        BufferAllocator<OwnerClass, type, PageSizeKB, MaxFragmentationOccupancy, MaxFreeBuffers, InitialBufferSize>;
    using RefTy = TypedBufferRef<OwnerClass>;

    ~BufferAllocator() override = default;

private:
    /// @brief A buffer in-flight is one that is being used by the GPU. Such buffers become free
    /// once their `frameSync` fence becomes signaled.
    struct InFlightBuffer {
        size_t bufferIndex; // Index into buffers list
        std::shared_ptr<gl::Fence> frameSync;

        InFlightBuffer(size_t buf, std::shared_ptr<gl::Fence> sync)
            : bufferIndex(buf),
              frameSync(std::move(sync)) {}
    };

public:
    /// @brief An allocated buffer the allocator manages. These buffers are sub-allocated
    /// in monotonic fashion. Once a buffer's capacity has been reached, it is stored until
    /// it can be recycled and a new buffer is acquired to replace it.
    struct Buffer {
        BufferTy& allocator;

        // The pointer points to the next unused region of memory in the buffer
        ptrdiff_t pointer = 0;

        // References to buffer objects allocating from this block of memory.
        // For performance reasons, we use a vector here. To deal with vector storage reallocation,
        // vector resizes are handled explicitly and Refs are made aware of relocation events.
        std::vector<RefTy> refs;

        // Backing GL object for this buffer
        GLuint id = 0;

        // Tombstones indicating a removed ref. When a Ref is removed, a tombstone is added to this
        // counter. This is much faster vs. erasing from our vector. This works because we enforce
        // a monotonic allocation scheme on our buffers and recycle the whole buffer at once.
        size_t tombstones = 0;

        // Used to drive defragmentation, occupancy is a measure of the number of bytes held by
        // living references
        size_t occupancyBytes = 0;

        // We need to know the index to ourselves in the main buffer list
        size_t bufferIndex = 0;

        // If true, the buffer's memory has been released to compact memory
        bool reclaimed = false;

        Buffer(BufferAllocator& allocator_)
            : allocator(allocator_) {
            refs.reserve(InitialBufferSize);
            MBGL_CHECK_ERROR(glGenBuffers(1, &id));
            MBGL_CHECK_ERROR(glBindBuffer(type, id));
            MBGL_CHECK_ERROR(glBufferData(type, PageSize, nullptr, GL_DYNAMIC_DRAW));
        }

        ~Buffer() {
            if (id != 0) {
                glDeleteBuffers(1, &id);
                id = 0;
            }
        }

        void reclaim() {
            refs.clear();
            refs = decltype(refs)();

            if (id != 0) {
                glDeleteBuffers(1, &id);
                id = 0;
            }
        }

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& rhs) noexcept
            : allocator(rhs.allocator),
              pointer(rhs.pointer),
              refs(std::move(rhs.refs)),
              id(rhs.id),
              tombstones(rhs.tombstones),
              occupancyBytes(rhs.occupancyBytes),
              bufferIndex(rhs.bufferIndex) {
            rhs.id = 0;
        }

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&& rhs) noexcept {
            allocator = rhs.allocator;
            pointer = rhs.pointer;
            refs = std::move(rhs.refs);
            id = rhs.id;
            rhs.id = 0;
            tombstones = rhs.tombstones;
            occupancyBytes = rhs.occupancyBytes;
            bufferIndex = rhs.bufferIndex;
            return *this;
        }

        void setBufferIndex(size_t index) noexcept { bufferIndex = index; }

        // Reset this buffer for a fresh recording session
        void reset() {
            assert(refs.size() - tombstones == 0);
            pointer = tombstones = occupancyBytes = 0;
            refs.clear();
        }

        /// @brief Once the buffer is full, we mark it 'in-flight', storing it in another location
        /// in the allocator. The defragmentation routine will traverse this location for buffers
        /// that can be recycled.
        /// @param index The buffer index that maps to this buffer instance.
        void markInFlight(size_t index) noexcept { allocator.inFlightBuffers.emplace_back(index); }

        /// @brief Number of living references to this buffer
        size_t numRefs() const noexcept { return refs.size() - tombstones; }

        /// @brief Add a living reference to a sub-allocated region of this buffer
        /// @param bufObj Managed instance that owns this sub-allocation
        /// @param bufPtr Offset into the buffer the allocation begins at
        /// @param size_ Size of the allocated region
        /// @return BufferRef instance
        RefTy* addRef(OwnerClass* bufObj, ptrdiff_t bufPtr, size_t size_) {
            // Important: Check if our list needs to reallocate, if so we need to handle it manually
            // so we can update the references with their new memory locations
            if (refs.size() == refs.capacity()) {
                auto oldList = std::exchange(refs, {});
                refs.reserve(oldList.size() * 2);

                for (size_t i = 0; i < oldList.size(); ++i) {
                    if (!oldList[i].getOwner()) {
                        // Released ref, skip it
                        continue;
                    }

                    // Relocate the reference
                    refs.push_back(std::move(oldList[i]));
                    refs.back().getOwner()->getManagedBuffer().relocRef(&refs.back());
                }

                // Since we dropped released refs, we can clear the tombstone count
                tombstones = 0;
            }

            refs.emplace_back(bufObj, bufferIndex, bufPtr, size_);
            occupancyBytes += size_;
            return &refs.back();
        }

        /// @brief Remove a living reference from this buffer.
        /// @note: Remember buffers are only permitted to grow monotonically. A buffer can only be
        /// reused once all living references are removed or relocated to other buffers.
        /// @param bufObj BufferRef to remove
        void decRef(RefTy* bufObj) {
#ifndef NDEBUG
            assert(bufObj->getOwner());
            // Sanity check this ref actually belongs
            auto it = std::find(refs.begin(), refs.end(), *bufObj);
            if (it != refs.end()) {
                tombstones++;
            } else {
                assert(0);
            }
#else
            tombstones++;
#endif
            bufObj->setOwner(nullptr);

            assert(occupancyBytes >= bufObj->getSize());
            occupancyBytes -= bufObj->getSize();
        }

        // Align a pointer on a set block size
        static size_t align(size_t ptr, size_t alignment) {
            if (ptr == alignment) {
                return ptr;
            }
            return ((ptr + alignment) / alignment) * alignment;
        }
    };

public:
    // Write a block of memory to the recording buffer
    // Returns the base index of written memory in the buffer
    bool write(const void* data, size_t size, BufferRef*& residentBuffer) noexcept override {
        Buffer* buffer = nullptr;
        if (buffers.size() == 0) {
            // Query for the alignment we must use
            int32_t alignment = 0;
            MBGL_CHECK_ERROR(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment));

            sharedBufferAlignment = static_cast<size_t>(alignment);
            recordingBuffer = newBuffer();
        }
        buffer = &buffers[recordingBuffer];

        const auto alignedSize = Buffer::align(size, sharedBufferAlignment);

        // Reached past the buffer range
        if ((buffer->pointer + alignedSize) > PageSize) {
            // Put the buffer in the pending list
            buffer->markInFlight(recordingBuffer);
            buffer = nullptr;

            // Look for a free buffer
            const auto freeIndex = getFreeBuffer();
            if (freeIndex != -1) {
                recordingBuffer = freeIndex;
            } else {
                // Otherwise, allocate a fresh buffer
                recordingBuffer = newBuffer();
            }
            buffer = &buffers[recordingBuffer];
        }

        assert(buffer);
        if (!buffer) {
            assert(0);
            return false;
        }
        MBGL_CHECK_ERROR(glBindBuffer(type, buffer->id));

        // Map the next available slice of memory
        auto* buf = MBGL_CHECK_ERROR(
            glMapBufferRange(type,
                             buffer->pointer,
                             alignedSize,
                             GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT));
        const auto writtenIndex = buffer->pointer;

        if (buf) {
            std::memcpy(buf, data, size);
            MBGL_CHECK_ERROR(glUnmapBuffer(type));

            residentBuffer = buffer->addRef(nullptr, writtenIndex, size);
            buffer->pointer += alignedSize;
        } else {
            assert(0);
            return false;
        }

#ifndef NDEBUG
        MBGL_CHECK_ERROR(glBindBuffer(type, 0));
#endif

        return true;
    }

    /// Release a living reference
    void release(BufferRef* ref) noexcept override {
        assert(ref);
        if (!ref) {
            return;
        }
        buffers[ref->getBufferIndex()].decRef(static_cast<RefTy*>(ref));
    }

    // Look for buffers that either have zero living references or equal or under FragmentationThresh
    // references. In the latter case, attempt to relocate these allocations to more utilized buffers.
    void defragment(const std::shared_ptr<gl::Fence>& fence) override {
        MLN_TRACE_FUNC();
        if (buffers.size() == 0) {
            return;
        }
        intptr_t recycleIndex = recordingBuffer;

        // Phase one: Mark free buffers
        updateInFlight();

        // Phase two: Consolidate fragmentation
        MBGL_CHECK_ERROR(glBindBuffer(type, buffers[recycleIndex].id));

        for (auto it = inFlightBuffers.begin(); it != inFlightBuffers.end();) {
            auto& fragBuffer = buffers[*it];
            if (fragBuffer.occupancyBytes > FragmentedOccupancyLevel) {
                ++it;
                continue;
            }

            // This buffer is fragmented. Move allocations from this buffer and append to fresh ones
            // so that this one may be recycled.

            // For each ref, append to new buffer and update that ref
            for (auto refIt = fragBuffer.refs.begin(); refIt != fragBuffer.refs.end();) {
                if (!refIt->getOwner()) {
                    ++refIt;
                    continue; // This is a released reference
                }

                const auto alignedSize = Buffer::align(refIt->getOwner()->getSize(), sharedBufferAlignment);

                // 2.a: Get recycle buffer
                if (recycleIndex == -1 || (buffers[recycleIndex].pointer + alignedSize) > PageSize) {
                    if (recycleIndex != -1) {
                        // We filled this one up, mark it in-flight and get a fresh one.
                        buffers[recycleIndex].markInFlight(recycleIndex);
                    }

                    // We need to get a new buffer.
                    recycleIndex = recordingBuffer = getFreeBuffer();
                    if (recycleIndex == -1) {
                        // No more buffers available, we're done.
                        break;
                    }

                    MBGL_CHECK_ERROR(glBindBuffer(type, buffers[recycleIndex].id));
                }

                // 2.b: Copy into recycle buffer
                if (!moveRef(*refIt, recycleIndex, alignedSize)) {
                    // Relocation failure, stop trying to defrag.
#ifndef NDEBUG
                    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#endif
                    return;
                }
            }

            // All refs have relocated, clear this buffer.
            if (fragBuffer.numRefs() == 0) {
                fragBuffer.reset();
                // Store this buffer in a waiting area until the GPU is done using it
                waitingFree.emplace_back(fragBuffer.bufferIndex, fence);
                it = inFlightBuffers.erase(it);
            } else {
                // We only managed a partial (or no) defrag. Do nothing with this buffer.
                ++it;
            }

            // No more buffers to write into, stop.
            if (recycleIndex == -1) {
                break;
            }
        }

#ifndef NDEBUG
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#endif

        if (recordingBuffer == -1) {
            recordingBuffer = newBuffer();
        }
    }

    size_t pageSize() const noexcept override { return PageSize; }

    int32_t getBufferID(size_t bufferIndex) const noexcept override { return buffers[bufferIndex].id; }

private:
    size_t newBuffer() noexcept {
        buffers.emplace_back(*this);
        const auto index = recordingBuffer = buffers.size() - 1;
        buffers[index].setBufferIndex(index);
        return index;
    }

    intptr_t getFreeBuffer() {
        if (freeList.size() > 0) {
            assert(buffers[freeList.back()].pointer == 0);
            const auto index = freeList.back();
            freeList.pop_back();
            return index;
        } else if (reclaimedList.size() > 0) {
            const auto index = reclaimedList.back();
            reclaimedList.pop_back();
            new (&buffers[index]) Buffer(*this);
            buffers[index].setBufferIndex(index);
            return index;
        }
        return -1;
    }

    // Check waiting in-flight buffers and move them to the free list
    void updateInFlight() {
        for (auto it = waitingFree.begin(); it != waitingFree.end();) {
            if (it->frameSync->isSignaled()) {
                freeList.push_back(it->bufferIndex);
                it = waitingFree.erase(it);
            } else {
                ++it;
            }
        }

        // And check the free list for an excess of buffers, reclaim if we need to
        while (freeList.size() > MaxFreeBuffers) {
            reclaimedList.push_back(freeList.back());
            freeList.pop_back();
            buffers[reclaimedList.back()].reclaim();
        }
    }

    // Move a buffer reference (An allocation from a UniformBufferGL instance) to a new buffer
    bool moveRef(RefTy& ref, size_t toIndex, size_t alignedSize) {
        auto& destBuffer = buffers[toIndex];
        const auto recycledWriteIndex = destBuffer.pointer;

        auto* buf = MBGL_CHECK_ERROR(
            glMapBufferRange(type,
                             recycledWriteIndex,
                             alignedSize,
                             GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT));

        if (buf) {
            std::memcpy(buf, ref.getOwner()->getManagedBuffer().getContents().data(), ref.getOwner()->getSize());
            MBGL_CHECK_ERROR(glUnmapBuffer(type));
            destBuffer.pointer += alignedSize;
        } else {
            assert(0);
            return false;
        }

        // 2.c: Now the ref must be made aware of the relocation of its contents.
        // Note that this means defragmentation can never run on refs that are
        // expected to remain bound after defragmentation. They must be re-bound
        // to become valid again.
        auto owner = ref.getOwner();
        const auto refSize = ref.getSize();
        const auto oldIndex = ref.getBufferIndex();

        const auto newRef = destBuffer.addRef(owner, recycledWriteIndex, refSize);
        buffers[oldIndex].decRef(&ref);
        owner->getManagedBuffer().relocRef(newRef);
        return true;
    }

private:
    // GL requires us to satisfy this alignment for sub-allocations in our buffers
    size_t sharedBufferAlignment = 0;
    // Our active recording buffer
    intptr_t recordingBuffer = 0;

    // Buffers free for re-use
    std::vector<size_t> freeList;
    // Buffers reclaimed that can be recreated if needed
    std::vector<size_t> reclaimedList;
    // Buffers recorded and currently in-use
    std::list<size_t> inFlightBuffers;
    // When we recycle a buffer, we copy from an in-flight buffer to a fresh one.
    // Before we can convert the in-flight buffer to a free one however, we must wait
    // after the copy operation(s) to ensure the GPU is done with it before we reuse it.
    std::list<InFlightBuffer> waitingFree;

public:
    // All buffers allocated so far
    std::vector<Buffer> buffers;

    friend Buffer;
};

class UniformBufferAllocator::Impl : public gl::BufferAllocator<gl::UniformBufferGL, GL_UNIFORM_BUFFER> {};

UniformBufferAllocator::~UniformBufferAllocator() = default;

UniformBufferAllocator::UniformBufferAllocator() {
    impl = std::make_unique<UniformBufferAllocator::Impl>();
}

bool UniformBufferAllocator::write(const void* data, size_t size, BufferRef*& ref) noexcept {
    return impl->write(data, size, ref);
}

void UniformBufferAllocator::release(BufferRef* ref) noexcept {
    impl->release(ref);
}

void UniformBufferAllocator::defragment(const std::shared_ptr<gl::Fence>& fence) {
    impl->defragment(fence);
}

size_t UniformBufferAllocator::pageSize() const noexcept {
    return impl->pageSize();
}

int32_t UniformBufferAllocator::getBufferID(size_t bufferIndex) const noexcept {
    return impl->getBufferID(bufferIndex);
}

} // namespace gl
} // namespace mbgl
