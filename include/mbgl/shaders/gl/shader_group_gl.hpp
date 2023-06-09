#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/programs/program_parameters.hpp>

namespace mbgl {
namespace gl {

template <shaders::BuiltIn ShaderID>
class ShaderGroupGL final : public gfx::ShaderGroup {
public:
    ShaderGroupGL(const ProgramParameters& programParameters_)
        : ShaderGroup(),
          programParameters(programParameters_){};
    ~ShaderGroupGL() noexcept override = default;

    const std::shared_ptr<gfx::Shader> getOrCreateShader(
        gfx::Context& context, const std::vector<std::string>& propertiesAsUniforms) override {
        const auto& name = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::name;
        const auto& vert = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::vertex;
        const auto& frag = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::fragment;

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
        std::string shaderName = std::string(name) + "#" + std::to_string(key);
        auto shader = get<gl::ShaderProgramGL>(shaderName);
        if (!shader) {
            shader = ShaderProgramGL::create(
                static_cast<gl::Context&>(context), programParameters, shaderName, vert, frag, additionalDefines);
            if (!registerShader(shader, shaderName)) {
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }
        }
        return shader;
    }

private:
    ProgramParameters programParameters;
};

} // namespace gl
} // namespace mbgl
