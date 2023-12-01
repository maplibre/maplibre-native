#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/compression.hpp>
#include <mbgl/util/logging.hpp>

#include <vector>
#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

namespace {

template<size_t PageSizeKB = 8ULL, size_t FragmentationThresh = 3ULL>
struct BufferAllocator {
public:
    static constexpr const auto PageSize = 1024 * PageSizeKB;

private:
    struct InFlightBuffer {
        size_t bufferIndex; // Index into buffers list
        std::shared_ptr<gl::Fence> frameSync;

        InFlightBuffer(size_t buf, std::shared_ptr<gl::Fence>&& sync)
            : bufferIndex(buf),
            frameSync(sync) {}
    };

public:
    // A buffer is a GL name and reference count
    struct Buffer {
        BufferAllocator& allocator;
        // The pointer points to the next unused region of memory in the buffer
        ptrdiff_t pointer = 0;
        // References to buffer objects allocating from this block of memory
        std::vector<UniformBufferGL::Ref> refs;
        // Backing GL object for this buffer
        GLuint id = 0;
        // Tombstones indicating a removed ref
        size_t tombstones = 0;
        
        Buffer(BufferAllocator& allocator_) : allocator(allocator_) {
            refs.reserve(128);

            MBGL_CHECK_ERROR(glGenBuffers(1, &id));
            MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, id));
            MBGL_CHECK_ERROR(glBufferData(GL_UNIFORM_BUFFER, PageSize, nullptr, GL_DYNAMIC_DRAW));
        }

        ~Buffer() {
            if (id != 0) {
                glDeleteBuffers(1, &id);
                id = 0;
            }
        }

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& rhs) noexcept : allocator(rhs.allocator) {
            id = rhs.id;
            rhs.id = 0;

            refs = std::move(rhs.refs);
            pointer = rhs.pointer;
        }

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&& rhs) noexcept {
            allocator = rhs.allocator;
            id = rhs.id;
            rhs.id = 0;

            refs = std::move(rhs.refs);
            pointer = rhs.pointer;
            return *this;
        }

        // Reset this buffer for a fresh recording session
        void reset() {
            assert(refs.size() - tombstones == 0);
            pointer = tombstones = 0;
            refs.clear();
        }

        void markInFlight(size_t index) noexcept {
            allocator.inFlightBuffers.emplace_back(index);
        }

        size_t numRefs() const noexcept {
            return refs.size() - tombstones;
        }

        UniformBufferGL::Ref* addRef(UniformBufferGL* bufObj, ptrdiff_t bufPtr, size_t size_) {
            // Important: Check if our list needs to reallocate, if so we need to handle it manually
            // so we can update the references with their new memory locations
            if (refs.size() == refs.capacity()) {
                auto oldList = std::move(refs);
                refs.reserve(oldList.size() * 2);

                for (size_t i = 0; i < oldList.size(); ++i) {
                    if (!oldList[i].refPtr) {
                        continue;
                    }

                    refs.push_back(std::move(oldList[i]));
                    refs.back().refPtr->relocRef(&refs.back());
                }

                tombstones = 0;
            }

            refs.emplace_back(bufObj, bufPtr, size_);
            return &refs.back();
        }

        void decRef(UniformBufferGL::Ref*& bufObj) {
            auto it = std::find(refs.begin(), refs.end(), *bufObj);
            if (it != refs.end()) {
                it->refPtr = nullptr;
                bufObj = nullptr;
                tombstones++;
            }
        }

        // Align a pointer on a set block size
        static size_t align(size_t ptr, size_t alignment) {
            return ((ptr + alignment) / alignment) * alignment;
        }
    };

public:
    // Write a block of memory to the recording buffer
    // Returns the base index of written memory in the buffer
    // Outputs the buffer index that was used for lifetime management
    ptrdiff_t write(const void* data, size_t size, size_t& residentBuffer) {
        Buffer* buffer = nullptr;
        if (buffers.size() == 0) {
            // Query for the alignment we must use
            int32_t alignment = 0;
            MBGL_CHECK_ERROR(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment));
            sharedBufferAlignment = static_cast<size_t>(alignment);
            recordingBuffer = 0;
            buffers.emplace_back(*this);
            buffer = &buffers.back();
        } else {
            buffer = &buffers[recordingBuffer];
        }

        const auto alignedSize = Buffer::align(size, sharedBufferAlignment);

        // Reached past the buffer range
        if (static_cast<size_t>(buffer->pointer + alignedSize) > PageSize) {
            // Put the buffer and fence in the pending list
            buffer->markInFlight(recordingBuffer);
            buffer = nullptr;

            // Look for a free buffer
            if (freeList.size() > 0) {
                recordingBuffer = freeList.back();
                freeList.pop_back();
                buffer = &buffers[recordingBuffer];
            } else {
                // Check if any in-flight buffers have signaled
                for (auto it = inFlightBuffers.begin(); it != inFlightBuffers.end(); ++it) {
                    if (buffers[*it].numRefs() == 0) {
                        recordingBuffer = *it;
                        buffers[recordingBuffer].reset();
                        inFlightBuffers.erase(it);
                        buffer = &buffers[recordingBuffer];
                        break;
                    }
                }
            }

            // Otherwise, allocate a fresh buffer
            if (!buffer) {
                buffers.emplace_back(*this);
                recordingBuffer = buffers.size() - 1;
                buffer = &buffers[recordingBuffer];
            }
        }

        assert(buffer);
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, buffer->id));

        // Map the next available slice of memory
        auto* buf = MBGL_CHECK_ERROR(
            glMapBufferRange(GL_UNIFORM_BUFFER,
                buffer->pointer,
                alignedSize,
                GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT));
        const auto writtenIndex = buffer->pointer;

        if (buf) {
            std::memcpy(buf, data, size);
            MBGL_CHECK_ERROR(glUnmapBuffer(GL_UNIFORM_BUFFER));
            residentBuffer = recordingBuffer;
            buffer->pointer += alignedSize;
        }
        else {
            assert(0);
        }

#ifndef NDEBUG
        MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#endif

        return writtenIndex;
    }

    void defragment(const gl::Context& context) {
        intptr_t recycleIndex = -1;
        ptrdiff_t recyclePtr = 0;

        // Phase one: Mark free buffers
        updateInFlight();

        // Phase two: Consolidate fragmentation
        for (auto it = inFlightBuffers.begin(); it != inFlightBuffers.end();) {
            auto& fragBuffer = buffers[*it];
            if (fragBuffer.numRefs() == 0 && fragBuffer.numRefs() > FragmentationThresh) {
                ++it;
                continue;
            }

            // This buffer is fragmented. Move allocations from this buffer and append to fresh ones
            // so that this one may be recycled.

            // For each ref, append to new buffer and update that ref
            for (auto refIt = fragBuffer.refs.begin(); refIt != fragBuffer.refs.end();) {
                // 2.a: Get recycle buffer
                if (recycleIndex == -1 || recyclePtr + refIt->size > PageSize) {
                    if (recycleIndex != -1) {
                        // We filled this one up, mark it in-flight and get a fresh one.
                        buffers[recycleIndex].markInFlight(recycleIndex);
                    }

                    // We need to get a new buffer.
                    recycleIndex = getFreeBuffer(false);
                    if (recycleIndex == -1) {
                        // No more buffers available, we're done.
                        break;
                    }

                    recyclePtr = 0;
                    MBGL_CHECK_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, buffers[recycleIndex].id));
                }

                // 2.b: Copy into recycle buffer
                moveRef(*refIt, recycleIndex);
                refIt = fragBuffer.refs.erase(refIt);
            }

            // All refs have relocated, clear this buffer.
            if (fragBuffer.numRefs() == 0) {
                fragBuffer.reset();
                // Store this buffer in a waiting area until the GPU is done using it
                waitingFree.emplace_back(*it, context.getCurrentFrameFence());
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
    }

private:
    size_t getFreeBuffer(bool permitAllocation = true) {
        if (freeList.size() > 0) {
            intptr_t index = static_cast<intptr_t>(freeList.back());
            freeList.pop_back();
            return index;
        }

        if (permitAllocation) {
            // We must allocate.
            buffers.emplace_back(*this);
            return static_cast<intptr_t>(buffers.size()) - 1;
        }
        else {
            return -1;
        }
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
    }

    // Move a buffer reference (An allocation from a UniformBufferGL istance) to a new buffer
    void moveRef(UniformBufferGL::Ref& ref, size_t toIndex) {
        auto& destBuffer = buffers[toIndex];
        const auto recycledWriteIndex = destBuffer.pointer;
        const auto alignedSize = Buffer::align(ref.refPtr->getSize(), sharedBufferAlignment);

        auto* buf = MBGL_CHECK_ERROR(
            glMapBufferRange(GL_UNIFORM_BUFFER,
                recycledWriteIndex,
                alignedSize,
                GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT));

        if (buf) {
            std::memcpy(buf, ref.refPtr->getContents().data(), ref.refPtr->getSize());
            MBGL_CHECK_ERROR(glUnmapBuffer(GL_UNIFORM_BUFFER));
            destBuffer.pointer += alignedSize;
        } else {
            assert(0);
        }

        // 2.c: Now the ref must be made aware of the relocation of its contents.
        // Note that this means defragmentation can never run on refs that are
        // expected to remain bound after defragmentation. They must be re-bound
        // to become valid again.
        ref.refPtr->relocBuffer(toIndex, recycledWriteIndex);
    }

private:
    // GL requires us to satisfy this alignment for sub-allocations in our buffers
    size_t sharedBufferAlignment = 0;
    // Our active recording buffer
    size_t recordingBuffer = 0;

    // Buffers free for re-use
    std::vector<size_t> freeList;
    // Buffers recorded and currently in-use
    std::vector<size_t> inFlightBuffers;
    // When we recycle a buffer, we copy from an in-flight buffer to a fresh one.
    // Before we can convert the in-flight buffer to a free one however, we must wait
    // after the copy operation(s) to ensure the GPU is done with it before we reuse it.
    std::vector<InFlightBuffer> waitingFree;

public:
    // All buffers allocated so far
    std::vector<Buffer> buffers;

    friend Buffer;
};

static BufferAllocator<8, 3> allocator;

} // namespace

// Over time, buffers can accumulate long-lived allocations that prevent recycling.
// Look for these fragmented buffers and consolidate them.
void UniformBufferGL::defragment(const gl::Context& context) {
    allocator.defragment(context);
}

UniformBufferGL::UniformBufferGL(const void* data_, std::size_t size_)
    : UniformBuffer(size_) {
    if (size_ > decltype(allocator)::PageSize) {
        assert(0); // @TODO: Local allocation
    }

    alignedIndex = allocator.write(data_, size_, activeBuffer);
    contents.resize(size);
    std::memcpy(contents.data(), data_, size_);
    ref = allocator.buffers[activeBuffer].addRef(this, alignedIndex, size);
}

UniformBufferGL::UniformBufferGL(const UniformBufferGL& other)
    : UniformBuffer(other),
      ref(other.ref),
      contents(other.contents),
      activeBuffer(other.activeBuffer),
      alignedIndex(other.alignedIndex) {
    allocator.buffers[activeBuffer].addRef(this, alignedIndex, size);
}

UniformBufferGL::~UniformBufferGL() {
    allocator.buffers[activeBuffer].decRef(ref);
}

BufferID UniformBufferGL::getID() const {
    return allocator.buffers[activeBuffer].id;
}

void UniformBufferGL::relocBuffer(size_t bufferID, ptrdiff_t index) noexcept {
    activeBuffer = bufferID;
    alignedIndex = index;
}

void UniformBufferGL::relocRef(Ref* ref_) noexcept {
    ref = ref_;
}

void UniformBufferGL::update(const void* data_, std::size_t size_) {
    assert(size == size_);
    if (size != size_) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }

    allocator.buffers[activeBuffer].decRef(ref);
    alignedIndex = allocator.write(data_, size_, activeBuffer);
    std::memcpy(contents.data(), data_, size_);
    ref = allocator.buffers[activeBuffer].addRef(this, alignedIndex, size);
}

} // namespace gl
} // namespace mbgl
