#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/util/containers.hpp>

namespace mbgl {
namespace gl {

template <shaders::BuiltIn ShaderID>
class ShaderGroupGL final : public gfx::ShaderGroup {
public:
    ShaderGroupGL(const ProgramParameters& programParameters_)
        : gfx::ShaderGroup(),
          programParameters(programParameters_.withProgramType(ShaderID)) {}
    ~ShaderGroupGL() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& context,
                                     const StringIDSetsPair& propertiesAsUniforms,
                                     std::string_view firstAttribName) override {
        constexpr auto& name = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::name;
        constexpr auto& vert = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::vertex;
        constexpr auto& frag = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::fragment;

        // We could cache these by key here to avoid creating a string key each time, but we
        // would need another mutex.  We could also push string IDs down into `ShaderGroup`.
        std::size_t seed = 0;
        mbgl::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
        mbgl::util::hash_combine(seed, programParameters.getDefinesHash());
        const std::string shaderName = getShaderName(name, seed);

        auto shader = get<gl::ShaderProgramGL>(shaderName);
        if (shader) {
            return shader;
        }

        // No match, we need to create the shader.
        std::string additionalDefines;
        additionalDefines.reserve(propertiesAsUniforms.first.size() * 48);
        for (const auto propertyName : propertiesAsUniforms.first) {
            // We expect the names to be prefixed by "a_", but we need just the base here.
            const auto* prefix = propertyName.data();
            if (prefix[0] == 'a' && prefix[1] == '_') {
                prefix += 2;
            }

            additionalDefines += "#define HAS_UNIFORM_u_";
            additionalDefines += prefix;
            additionalDefines += "\n";
        }

        auto& glContext = static_cast<gl::Context&>(context);
        shader = ShaderProgramGL::create(glContext,
                                         programParameters,
                                         firstAttribName,
                                         shaders::ShaderInfo<ShaderID, gfx::Backend::Type::OpenGL>::uniformBlocks,
                                         shaders::ShaderInfo<ShaderID, gfx::Backend::Type::OpenGL>::textures,
                                         shaders::ShaderInfo<ShaderID, gfx::Backend::Type::OpenGL>::attributes,
                                         vert,
                                         frag,
                                         additionalDefines);
        if (!shader || !registerShader(shader, shaderName)) {
            throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
        }
        return shader;
    }

private:
    ProgramParameters programParameters;
};

} // namespace gl
} // namespace mbgl
