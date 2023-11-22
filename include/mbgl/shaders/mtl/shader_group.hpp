#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/containers.hpp>

#include <numeric>
#include <string>
#include <unordered_map>

namespace mbgl {
namespace mtl {

template <shaders::BuiltIn ShaderID>
class ShaderGroup final : public gfx::ShaderGroup {
public:
    ShaderGroup(const ProgramParameters& programParameters_)
        : gfx::ShaderGroup(),
          programParameters(programParameters_) {}
    ~ShaderGroup() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& gfxContext,
                                     const mbgl::unordered_set<StringIdentity>& propertiesAsUniforms,
                                     std::string_view /*firstAttribName*/) override {
        constexpr auto& name = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::name;
        constexpr auto& source = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::source;
        constexpr auto& vertMain = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::vertexMainFunction;
        constexpr auto& fragMain = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::fragmentMainFunction;

        const std::string shaderName = std::string(name);

        auto shader = get<mtl::ShaderProgram>(shaderName);
        if (!shader) {
            auto& context = static_cast<Context&>(gfxContext);
            const auto shaderSource = std::string(shaders::prelude) + source;
            shader = context.createProgram(shaderName, shaderSource, vertMain, fragMain, programParameters, {});
            assert(shader);
            if (!shader || !registerShader(shader, shaderName)) {
                assert(false);
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }

            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>;
            for (const auto& attrib : ShaderClass::attributes) {
                shader->initAttribute(attrib);
            }
            for (const auto& uniform : ShaderClass::uniforms) {
                shader->initUniformBlock(uniform);
            }
            for (const auto& texture : ShaderClass::textures) {
                shader->initTexture(texture);
            }
        }
        return shader;
    }

private:
    ProgramParameters programParameters;
};

} // namespace mtl
} // namespace mbgl
