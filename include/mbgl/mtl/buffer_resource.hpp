#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLBuffer.hpp>
#include <Metal/MTLDevice.hpp>

namespace mbgl {
namespace mtl {

class BufferResource {
public:
    BufferResource() = default;
    /** @brief Create a new Metal buffer
        @param device The Metal device on which to create the buffer.
        @param raw Data to use for the contents of the new buffer.  May be null.
        @param size The minimum size of the new buffer.  Must be non-zero.
        @param usage A `MTL::ResourceOptions` value.  Currently, only `ResourceStorageModeShared` is supported.
     */
    BufferResource(MTLDevicePtr device, const void* raw, std::size_t size, MTL::ResourceOptions usage);
    BufferResource(const BufferResource&);
    BufferResource(BufferResource&&);
    BufferResource& operator=(BufferResource&&);

    void update(const void* data, std::size_t size, std::size_t offset);

    std::size_t getSizeInBytes() const { return buffer ? buffer->length() : 0; }
    void* contents() const { return buffer ? buffer->contents() : nullptr; }

    const MTLBufferPtr& getMetalBuffer() const { return buffer; }

    operator bool() const { return buffer.operator bool(); }

protected:
    MTLDevicePtr device;
    MTLBufferPtr buffer;
    NS::UInteger usage;
};

} // namespace mtl
} // namespace mbgl
