#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/program.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/program.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/platform/gl_functions.hpp>
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

    void draw(const PaintParameters& parameters, int32_t lineWidth) const {
        auto& glContext = static_cast<gl::Context&>(parameters.context);
        if (!triangleIndexes.empty()) {
            glContext.draw(gfx::Triangles(), lineIndexes.size(), triangleIndexes.size());
        }
        if (!lineIndexes.empty()) {
            glContext.draw(gfx::Lines(lineWidth), 0, lineIndexes.size());
        }
    }

    // ShaderID shaderId;
    // RenderbufferID renderTarget;
    std::vector<TextureID> textures;

    std::vector<std::uint16_t> lineIndexes;
    std::vector<std::uint16_t> triangleIndexes;

    VertexAttributeArrayGL vertexAttributes;

    VertexArray vertexArray = {{nullptr, false}};
    gfx::IndexBuffer indexBuffer = {0, nullptr};
    gfx::UniqueVertexBufferResource attributeBuffer;

    UniformBufferArrayGL uniformBuffers;

    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::ColorMode colorMode;
    gfx::CullFaceMode cullFaceMode;
    // gfx::UniformValues<typename Program<>::UniformList> uniformValues;
    // Segment<Program<>::AttributeList> segment;
    // gfx::AttributeBindings<Program<>::AttributeList> attributeBindings;
    // gfx::TextureBindings<Program<>::TextureList> textureBindings;
    GLfloat pointSize = 0.0f;
};

} // namespace gl
} // namespace mbgl
