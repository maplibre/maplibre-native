#pragma once

#include <mln/gfx/drawable_impl.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/gfx/uniform.hpp>
#include <mln/gl/defines.hpp>
#include <mln/gl/enum.hpp>
#include <mln/gl/uniform_buffer_gl.hpp>
#include <mln/gl/vertex_array.hpp>
#include <mln/platform/gl_functions.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mln {
namespace gl {

using namespace platform;

class DrawableGL::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    std::vector<TextureID> textures;

    gfx::IndexVectorBasePtr indexes;

    std::vector<std::uint8_t> vertexData;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = static_cast<gfx::AttributeDataType>(-1);

    AttributeBindingArray attributeBindings;
    std::vector<gfx::UniqueVertexBufferResource> attributeBuffers;

    UniformBufferArrayGL uniformBuffers;

    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::CullFaceMode cullFaceMode;
    GLfloat pointSize = 0.0f;

    size_t vertexAttrId = 0;
};

struct DrawableGL::DrawSegmentGL final : public gfx::Drawable::DrawSegment {
    DrawSegmentGL(gfx::DrawMode mode_, SegmentBase&& segment_, VertexArray&& vertexArray_)
        : gfx::Drawable::DrawSegment(mode_, std::move(segment_)),
          vertexArray(std::move(vertexArray_)) {}

    ~DrawSegmentGL() override = default;

    const VertexArray& getVertexArray() const { return vertexArray; }
    void setVertexArray(VertexArray&& value) { vertexArray = std::move(value); }

protected:
    VertexArray vertexArray;
};

} // namespace gl
} // namespace mln
