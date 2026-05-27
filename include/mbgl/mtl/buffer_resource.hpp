#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLBuffer.hpp>
#include <Metal/MTLDevice.hpp>

namespace mbgl {
namespace mtl {

class Context;

class BufferResource {
public:
    BufferResource() noexcept = delete;
    /** @brief Create a new Metal buffer
        @param device The Metal device on which to create the buffer.
        @param raw Data to use for the contents of the new buffer.  May be null.
        @param size The minimum size of the new buffer.  Must be non-zero.
        @param usage A `MTL::ResourceOptions` value.  Currently, only `ResourceStorageModeShared` is supported.
        @param isIndexBuffer True if the buffer will be used for indexes.  The Metal API only accepts `MTLBuffer`
       objects for drawing indexed primitives, so this constrains how the buffer can be managed.
        @param persistent Performance hint, assume this buffer will be reused many times.
     */
    BufferResource(Context& context_,
                   const void* raw,
                   std::size_t size,
                   MTL::ResourceOptions usage,
                   bool isIndexBuffer,
                   bool persistent);
    BufferResource(BufferResource&&) noexcept;
    virtual ~BufferResource() noexcept;

    BufferResource& operator=(BufferResource&&) noexcept;

    BufferResource clone() const;

    void update(const void* data, std::size_t size, std::size_t offset) noexcept;

    std::size_t getSizeInBytes() const noexcept { return size; }
    const void* contents() const noexcept { return buffer ? buffer->contents() : (raw.empty() ? nullptr : raw.data()); }

    Context& getContext() const noexcept { return context; }
    const MTLBufferPtr& getMetalBuffer() const noexcept { return buffer; }

    bool isValid() const noexcept { return buffer || !raw.empty(); }
    operator bool() const noexcept { return isValid(); }
    bool operator!() const noexcept { return !isValid(); }

    using VersionType = std::uint16_t;

    /// Used to detect whether buffer contents have changed
    VersionType getVersion() const noexcept { return version; }

    /// Indicates whether this buffer needs to be re-bound from a previous binding at the given version
    bool needReBind(VersionType version) const noexcept;

    /// Bind this buffer to the specified vertex buffer index
    void bindVertex(const MTLRenderCommandEncoderPtr&,
                    std::size_t offset,
                    std::size_t index,
                    std::size_t size) const noexcept;
    /// Bind this buffer to the specified fragment buffer index
    void bindFragment(const MTLRenderCommandEncoderPtr&,
                      std::size_t offset,
                      std::size_t index,
                      std::size_t size) const noexcept;

    /// Update the offset, when this buffer is alread bound to the specified index (unchecked).
    void updateVertexBindOffset(const MTLRenderCommandEncoderPtr&,
                                std::size_t offset,
                                std::size_t index,
                                std::size_t size) const noexcept;
    void updateFragmentBindOffset(const MTLRenderCommandEncoderPtr&,
                                  std::size_t offset,
                                  std::size_t index,
                                  std::size_t size) const noexcept;

protected:
    Context& context;
    MTLBufferPtr buffer;
    std::vector<std::uint8_t> raw;
    NS::UInteger size;
    NS::UInteger usage;
    std::uint16_t version = 0;
    bool isIndexBuffer;
    bool persistent;

    mutable bool usingVertexBuffer = false;
};

} // namespace mtl
} // namespace mbgl
