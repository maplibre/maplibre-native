#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_manifest.hpp>

#include <numeric>
#include <string>
#include <unordered_map>

namespace mbgl {
namespace mtl {

class ShaderGroup final : public gfx::ShaderGroup {
public:
    ShaderGroup(shaders::BuiltIn programID_, const ProgramParameters& programParameters_)
        : gfx::ShaderGroup(),
          programID(programID_),
          programParameters(programParameters_) {}
    ~ShaderGroup() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& gfxContext,
                                     const std::unordered_set<StringIdentity>& propertiesAsUniforms,
                                     std::string_view /*firstAttribName*/) override {
        const auto& reflectionData = mbgl::shaders::getReflectionData<gfx::Backend::Type::Metal>(programID);
        auto shader = get<mtl::ShaderProgram>(reflectionData.name);

        if (!shader) {
            auto& context = static_cast<Context&>(gfxContext);
            
            // TODO: Compile the prelude as a library and refer to links in reflection data
            const auto shaderSource =
                shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::Metal>::source() + "\n" +
                programParameters.vertexSource(gfx::Backend::Type::Metal); // TODO: Currently using vertex source for metal shaders
            
            shader = context.createProgram(reflectionData.name, std::move(shaderSource),
                reflectionData.vertexMainFunction, reflectionData.fragmentMainFunction,
                programParameters, {});
            
            assert(shader);
            if (!shader || !registerShader(shader, reflectionData.name)) {
                assert(false);
                throw std::runtime_error("Failed to register " + reflectionData.name + " with shader group!");
            }

            for (const auto& attrib : reflectionData.attributes) {
                shader->initAttribute(attrib);
            }
            for (const auto& uniform : reflectionData.uniforms) {
                shader->initUniformBlock(uniform);
            }
            for (const auto& texture : reflectionData.textures) {
                shader->initTexture(texture);
            }
        }
        return shader;
    }

private:
    shaders::BuiltIn programID;
    ProgramParameters programParameters;
};

} // namespace mtl
} // namespace mbgl
