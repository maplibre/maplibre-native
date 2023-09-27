#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/programs/program_parameters.hpp>

#include <unordered_set>

namespace mbgl {
namespace gl {

template <shaders::BuiltIn ShaderID>
class ShaderGroupGL final : public gfx::ShaderGroup {
public:
    ShaderGroupGL(const ProgramParameters& programParameters_)
        : ShaderGroup(),
          programParameters(programParameters_) {}
    ~ShaderGroupGL() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& context,
                                     const std::unordered_set<StringIdentity>& propertiesAsUniforms,
                                     std::string_view firstAttribName) override {
        constexpr auto& name = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::name;
        constexpr auto& vert = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::vertex;
        constexpr auto& frag = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::fragment;

        // quick-n-dirty order-indepdendent hash combine
        size_t sum = 0, product = 1;
        for (const auto nameID : propertiesAsUniforms) {
            constexpr auto somePrime = 1099511628211ll;
            sum += nameID;
            product *= nameID * somePrime;
        }
        const size_t key = sum + product;

        // We could cache these by key here to avoid creating a string key each time, but we
        // would need another mutex.  We could also push string IDs down into `ShaderGroup`.
        const std::string shaderName = std::string(name) + "#" + std::to_string(key);
        auto shader = get<gl::ShaderProgramGL>(shaderName);
        if (shader) {
            return shader;
        }

        // No match, we need to create the shader.
        std::string additionalDefines;
        additionalDefines.reserve(propertiesAsUniforms.size() * 48);
        for (const auto nameID : propertiesAsUniforms) {
            // We expect the names to be prefixed by "a_", but we need just the base here.
            const auto prefixedAttrName = StringIndexer::get(nameID);
            const auto* prefix = prefixedAttrName.data();
            if (prefix[0] == 'a' && prefix[1] == '_') {
                prefix += 2;
            }

            additionalDefines += "#define HAS_UNIFORM_u_";
            additionalDefines += prefix;
            additionalDefines += "\n";
        }

        auto& glContext = static_cast<gl::Context&>(context);
        shader = ShaderProgramGL::create(
            glContext, programParameters, shaderName, firstAttribName, vert, frag, additionalDefines);
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
