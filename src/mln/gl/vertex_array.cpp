#include <mln/gl/vertex_array.hpp>
#include <mln/gl/index_buffer_resource.hpp>
#include <mln/gl/context.hpp>

namespace mln {
namespace gl {

void VertexArray::bind(Context& context, const gfx::IndexBuffer& indexBuffer, const AttributeBindingArray& bindings) {
    context.bindVertexArray = state->vertexArray;
    state->indexBuffer = indexBuffer.getResource<gl::IndexBufferResource>().buffer;

    state->bindings.reserve(bindings.size());

    // NOLINTNEXTLINE(bugprone-too-small-loop-variable)
    for (AttributeLocation location = 0; location < bindings.size(); ++location) {
        if (state->bindings.size() <= location) {
            AttributeLocation loc = location;
            state->bindings.emplace_back(context, std::move(loc));
        }
        state->bindings[location] = bindings[location];
    }
}

} // namespace gl
} // namespace mln
