#pragma once

#include <mbgl/shaders/shader_program_base.hpp>

#include <optional>
#include <memory>

namespace NS {
template <class T> class SharedPtr;
} // namespace NS
namespace MTL {
class Function;
} // namespace MTL

namespace mbgl {
namespace mtl {

enum class ShaderType : uint32_t {
    Vertex = 0,
    Fragment = 1,
    Compute = 2,
    Mesh = 3,
};

class ShaderProgram;
using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    ShaderProgram(std::string name,
                  NS::SharedPtr<MTL::Function> vertexFunction,
                  NS::SharedPtr<MTL::Function> fragmentFunction);
    ~ShaderProgram() noexcept override;

    static constexpr std::string_view Name{"GenericMTLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    std::optional<uint32_t> getSamplerLocation(std::string_view name) const override;

    const gfx::UniformBlockArray& getUniformBlocks() const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override;

    gfx::UniformBlockArray& mutableUniformBlocks() override;

    gfx::VertexAttributeArray& mutableVertexAttributes() override;

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mtl
} // namespace mbgl
