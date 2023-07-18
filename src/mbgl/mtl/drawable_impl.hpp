#pragma once

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/program.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
//#include <mbgl/gl/defines.hpp>
//#include <mbgl/gl/enum.hpp>
//#include <mbgl/gl/program.hpp>
//#include <mbgl/gl/uniform_buffer_gl.hpp>
//#include <mbgl/gl/vertex_array.hpp>
//#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
namespace mtl {

using namespace platform;

class Drawable::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    /*
    std::vector<UniqueDrawSegment> segments;

    std::vector<TextureID> textures;
*/
    gfx::IndexVectorBasePtr indexes;
/*
    std::vector<std::uint8_t> vertexData;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = static_cast<gfx::AttributeDataType>(-1);
*/
    gfx::VertexAttributeArray vertexAttributes;

    gfx::IndexBuffer indexBuffer = {0, nullptr};
    std::vector<gfx::UniqueVertexBufferResource> attributeBuffers;

    gfx::UniformBufferArray uniformBuffers;

    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::CullFaceMode cullFaceMode;
    //GLfloat pointSize = 0.0f;
/*
    std::string vertexAttrName = "a_pos";
     */
};

//struct Drawable::DrawSegment final : public gfx::Drawable::DrawSegment {
//    DrawSegment(gfx::DrawMode mode_, SegmentBase&& segment_, VertexArray&& vertexArray_)
//        : gfx::Drawable::DrawSegment(mode_, std::move(segment_)),
//          vertexArray(std::move(vertexArray_)) {}
//    ~DrawSegment() override = default;
//
//    const VertexArray& getVertexArray() const { return vertexArray; }
//    void setVertexArray(VertexArray&& value) { vertexArray = std::move(value); }
//
//protected:
//    VertexArray vertexArray;
//};

} // namespace gl
} // namespace mbgl
