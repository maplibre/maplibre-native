#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gl/buffer_resource.hpp>

namespace mbgl {
namespace gl {

class IndexBufferResource : public BufferResource, public gfx::IndexBufferResource {
public:
    using BufferResource::AsyncAllocCallback;
    using BufferResource::AsyncUpdateCallback;

    // Create a buffer resource that takes ownership of buffer_
    IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_);

    // Create a non-owning buffer resource
    IndexBufferResource(AsyncAllocCallback alloc, AsyncUpdateCallback update, int byteSize_);

    ~IndexBufferResource() noexcept override;
};

} // namespace gl
} // namespace mbgl
