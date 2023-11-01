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
    BufferResource() = delete;
    /** @brief Create a new Metal buffer
        @param device The Metal device on which to create the buffer.
        @param raw Data to use for the contents of the new buffer.  May be null.
        @param size The minimum size of the new buffer.  Must be non-zero.
        @param usage A `MTL::ResourceOptions` value.  Currently, only `ResourceStorageModeShared` is supported.
     */
    BufferResource(Context& context_, const void* raw, std::size_t size, MTL::ResourceOptions usage);
    BufferResource(BufferResource&&) noexcept;
    virtual ~BufferResource();

    BufferResource& operator=(BufferResource&&) noexcept;

    BufferResource clone() const;

    void update(const void* data, std::size_t size, std::size_t offset) noexcept;

    std::size_t getSizeInBytes() const noexcept { return buffer ? buffer->length() : 0; }
    const void* contents() const noexcept { return buffer ? buffer->contents() : nullptr; }

    Context& getContext() const noexcept { return context; }
    const MTLBufferPtr& getMetalBuffer() const noexcept { return buffer; }

    operator bool() const noexcept { return buffer.operator bool(); }
    bool operator!() const noexcept { return !buffer.operator bool(); }

    void bindVertex(const MTLRenderCommandEncoderPtr&, std::size_t offset, std::size_t index) const noexcept;
    void bindFragment(const MTLRenderCommandEncoderPtr&, std::size_t offset, std::size_t index) const noexcept;

protected:
    Context& context;
    MTLBufferPtr buffer;
    NS::UInteger size;
    NS::UInteger usage;
};

} // namespace mtl
} // namespace mbgl
