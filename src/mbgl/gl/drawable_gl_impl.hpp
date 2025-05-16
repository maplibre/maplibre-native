#pragma once

#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
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
} // namespace mbgl
