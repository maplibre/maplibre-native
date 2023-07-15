#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/mtl/shader_program_mtl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/programs/program_parameters.hpp>

namespace mbgl {
namespace mtl {

template <shaders::BuiltIn ShaderID>
class ShaderGroupMTL final : public gfx::ShaderGroup {
public:
    ShaderGroupMTL(const ProgramParameters& programParameters_)
        : ShaderGroup(),
          programParameters(programParameters_){};
    ~ShaderGroupMTL() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& context,
                                     const std::vector<std::string>& propertiesAsUniforms,
                                     std::string_view firstAttribName) override {
        constexpr auto& name = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::name;
        constexpr auto& vert = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::vertex;
        constexpr auto& frag = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::fragment;

        uint32_t key = 0;
        std::string additionalDefines;
        for (unsigned int i = 0; i < propertiesAsUniforms.size(); i++) {
            if (propertiesAsUniforms[i].empty()) {
                continue;
            }
            key |= 1 << i;
            additionalDefines += "#define HAS_UNIFORM_u_";
            additionalDefines += propertiesAsUniforms[i];
            additionalDefines += "\n";
        }
        const std::string shaderName = std::string(name) + "#" + std::to_string(key);
        auto shader = get<mtl::ShaderProgramMTL>(shaderName);
        /*if (!shader) {
            auto& glContext = static_cast<gl::Context&>(context);
            shader = ShaderProgramMTL::create(
                glContext, programParameters, shaderName, firstAttribName, vert, frag, additionalDefines);
            if (!shader || !registerShader(shader, shaderName)) {
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }
        }*/
        return shader;
    }

private:
    ProgramParameters programParameters;
};

} // namespace mtl
} // namespace mbgl
