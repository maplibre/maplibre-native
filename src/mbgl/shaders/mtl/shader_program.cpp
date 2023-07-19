#include <mbgl/shaders/mtl/shader_program.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/uniform_block.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>

#include <Metal/Metal.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace mtl {

struct ShaderProgram::Impl {
    Impl(NS::SharedPtr<MTL::Function>&& vert,
         NS::SharedPtr<MTL::Function>&& frag) :
        vertexFunction(std::move(vert)),
        fragmentFunction(std::move(frag)) {}

    NS::SharedPtr<MTL::Function> vertexFunction;
    NS::SharedPtr<MTL::Function> fragmentFunction;
};

ShaderProgram::ShaderProgram(std::string name,
                             NS::SharedPtr<MTL::Function> vertexFunction,
                             NS::SharedPtr<MTL::Function> fragmentFunction)
    : ShaderProgramBase(),
      impl(std::make_unique<Impl>(std::move(vertexFunction),
                                  std::move(fragmentFunction))) {
}

ShaderProgram::~ShaderProgram() noexcept = default;

std::optional<uint32_t> ShaderProgram::getSamplerLocation(std::string_view name) const {
    return std::nullopt;
}

namespace {
static UniformBlockArray noUniforms;
static VertexAttributeArray noAttribs;
}
const gfx::UniformBlockArray& ShaderProgram::getUniformBlocks() const {
    return noUniforms;
}

const gfx::VertexAttributeArray& ShaderProgram::getVertexAttributes() const {
    return noAttribs;
}

gfx::UniformBlockArray& ShaderProgram::mutableUniformBlocks() {
    return noUniforms;
}

gfx::VertexAttributeArray& ShaderProgram::mutableVertexAttributes() {
    return noAttribs;
}

} // namespace mtl
} // namespace mbgl
