#pragma once

#include <mln/gfx/index_buffer.hpp>
#include <mln/gl/object.hpp>

namespace mln {
namespace gl {

class IndexBufferResource : public gfx::IndexBufferResource {
public:
    IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_);
    ~IndexBufferResource() noexcept override;

    UniqueBuffer buffer;
    int byteSize;
};

} // namespace gl
} // namespace mln
