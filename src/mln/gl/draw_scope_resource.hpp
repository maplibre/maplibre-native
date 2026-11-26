#pragma once

#include <mln/gfx/draw_scope.hpp>
#include <mln/gl/vertex_array.hpp>

namespace mln {
namespace gl {

class DrawScopeResource : public gfx::DrawScopeResource {
public:
    DrawScopeResource(VertexArray&& vertexArray_)
        : vertexArray(std::move(vertexArray_)) {}

    VertexArray vertexArray;
};

} // namespace gl
} // namespace mln
