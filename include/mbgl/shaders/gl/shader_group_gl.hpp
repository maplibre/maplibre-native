#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/util/hash.hpp>

#include <sstream>
#include <unordered_set>

namespace mbgl {
namespace gl {

class ShaderGroupGL final : public gfx::ShaderGroup {
public:
    ShaderGroupGL(shaders::BuiltIn programID_, const ProgramParameters& programParameters_)
        : ShaderGroup(),
          programID(programID_),
          programParameters(programParameters_) {}
    ~ShaderGroupGL() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& context,
                                     const std::unordered_set<StringIdentity>& propertiesAsUniforms,
                                     std::string_view firstAttribName) override {
        // Generate a map key for the specified combination of properties
        const size_t key = util::order_independent_hash(propertiesAsUniforms.begin(), propertiesAsUniforms.end());

        // We could cache these by key here to avoid creating a string key each time, but we
        // would need another mutex.  We could also push string IDs down into `ShaderGroup`.
        const std::string shaderName = (programID != shaders::BuiltIn::None
                                            ? std::string(shaders::getProgramName(programID))
                                            : std::to_string(reinterpret_cast<ptrdiff_t>(this)))
                                           .append(std::to_string(key)); // A user shader is just the instance address
        auto shader = get<gl::ShaderProgramGL>(shaderName);
        if (shader) {
            return shader;
        }

        // No match, we need to create the shader.
        std::string additionalDefines;
        additionalDefines.reserve(propertiesAsUniforms.size() * 48);
        for (const auto nameID : propertiesAsUniforms) {
            // We expect the names to be prefixed by "a_", but we need just the base here.
            const auto prefixedAttrName = stringIndexer().get(nameID);
            const auto* prefix = prefixedAttrName.data();
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
                                         shaderName,
                                         firstAttribName,
                                         programParameters.vertexSource(gfx::Backend::Type::OpenGL),
                                         programParameters.fragmentSource(gfx::Backend::Type::OpenGL),
                                         additionalDefines);
        if (!shader || !registerShader(shader, shaderName)) {
            throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
        }

        return shader;
    }

private:
    shaders::BuiltIn programID;
    ProgramParameters programParameters;
};

} // namespace gl
} // namespace mbgl
