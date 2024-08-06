#pragma once

#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/program.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/program.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
namespace gl {

using namespace platform;

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

struct IndexBufferGL : public gfx::IndexBufferBase {
    IndexBufferGL(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBufferGL() override = default;

    std::unique_ptr<mbgl::gfx::IndexBuffer> buffer;
};

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

    void createVAOs(gl::Context& context) {
        MLN_TRACE_FUNC();

        // Bind a VAO for each group of vertexes described by a segment
        for (const auto& seg : segments) {
            MLN_TRACE_ZONE(VAO_For_segment);
            auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
            const auto& mlSeg = glSeg.getSegment();

            if (mlSeg.indexLength == 0) {
                continue;
            }

            for (auto& binding : attributeBindings) {
                if (binding) {
                    binding->vertexOffset = static_cast<uint32_t>(mlSeg.vertexOffset);
                }
            }

            if (!glSeg.getVertexArray().isValid() && indexes) {
                auto vertexArray = context.createVertexArray();
                const auto& indexBuf = static_cast<IndexBufferGL&>(*indexes->getBuffer());
                vertexArray.bind(context, *indexBuf.buffer, attributeBindings);
                assert(vertexArray.isValid());
                if (vertexArray.isValid()) {
                    glSeg.setVertexArray(std::move(vertexArray));
                }
            }
        }
    }
};

} // namespace gl
} // namespace mbgl
