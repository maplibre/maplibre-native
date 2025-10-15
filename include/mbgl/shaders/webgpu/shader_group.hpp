#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/common.hpp>
#include <mbgl/shaders/webgpu/wgsl_preprocessor.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/webgpu/context.hpp>

#include <string>
#include <type_traits>
#include <unordered_map>

namespace mbgl {
namespace webgpu {

class ShaderGroupBase : public gfx::ShaderGroup {
protected:
    ShaderGroupBase(const ProgramParameters& parameters_)
        : programParameters(parameters_) {}

    using DefinesMap = mbgl::unordered_map<std::string, std::string>;
    void addAdditionalDefines(const StringIDSetsPair& propertiesAsUniforms, DefinesMap& additionalDefines) {
        additionalDefines.reserve(propertiesAsUniforms.first.size());
        for (const auto name : propertiesAsUniforms.first) {
            // We expect the names to be prefixed by "a_", but we need just the base here.
            const auto* base = (name[0] == 'a' && name[1] == '_') ? &name[2] : name.data();
            additionalDefines.insert(std::make_pair(std::string(uniformPrefix) + base, std::string()));
        }
    }

    ProgramParameters programParameters;

private:
    static constexpr auto uniformPrefix = "HAS_UNIFORM_u_";
};

template <shaders::BuiltIn ShaderID>
class ShaderGroup final : public ShaderGroupBase {
public:
    ShaderGroup(const ProgramParameters& programParameters_)
        : ShaderGroupBase(programParameters_) {}
    ~ShaderGroup() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& gfxContext,
                                     const StringIDSetsPair& propertiesAsUniforms,
                                     std::string_view /*firstAttribName*/) override {
        using ShaderSource = shaders::ShaderSource<ShaderID, gfx::Backend::Type::WebGPU>;
        constexpr auto& name = ShaderSource::name;
        constexpr auto& vert = ShaderSource::vertex;
        constexpr auto& frag = ShaderSource::fragment;

        std::size_t seed = 0;
        mbgl::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
        mbgl::util::hash_combine(seed, programParameters.getDefinesHash());
        const std::string shaderName = getShaderName(name, seed);

        auto shader = get<webgpu::ShaderProgram>(shaderName);
        if (!shader) {
            DefinesMap additionalDefines;
            addAdditionalDefines(propertiesAsUniforms, additionalDefines);

            std::string vertexSource;
            std::string fragmentSource;

            if constexpr (requires { ShaderSource::prelude; }) {
                vertexSource = std::string(ShaderSource::prelude) + vert;
                fragmentSource = std::string(ShaderSource::prelude) + frag;
            } else {
                vertexSource = vert;
                fragmentSource = frag;
            }

            std::unordered_map<std::string, bool> defineSet;
            const auto& programDefines = programParameters.getDefines();
            for (const auto& entry : programDefines) {
                defineSet.emplace(entry.first, true);
            }
            for (const auto& entry : additionalDefines) {
                defineSet.emplace(entry.first, true);
            }

            vertexSource = detail::preprocessWGSL(vertexSource, defineSet);
            fragmentSource = detail::preprocessWGSL(fragmentSource, defineSet);

            auto& context = static_cast<Context&>(gfxContext);
            shader = std::make_shared<ShaderProgram>(context,
                                                     vertexSource,
                                                     fragmentSource,
                                                     ShaderSource::attributes,
                                                     ShaderSource::textures,
                                                     std::string(name));

            assert(shader);
            if (!shader || !registerShader(shader, shaderName)) {
                assert(false);
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }
        }

        return shader;
    }
};

} // namespace webgpu
} // namespace mbgl
