#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/containers.hpp>

#include <numeric>
#include <string>
#include <type_traits>

namespace mbgl {
namespace vulkan {

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
        using ShaderSource = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Vulkan>;
        constexpr auto& name = ShaderSource::name;
        constexpr auto& prelude = ShaderSource::prelude;
        constexpr auto& vert = ShaderSource::vertex;
        constexpr auto& frag = ShaderSource::fragment;

        std::size_t seed = 0;
        mbgl::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
        mbgl::util::hash_combine(seed, programParameters.getDefinesHash());
        const std::string shaderName = getShaderName(name, seed);

        auto shader = get<vulkan::ShaderProgram>(shaderName);
        if (!shader) {
            DefinesMap additionalDefines;
            addAdditionalDefines(propertiesAsUniforms, additionalDefines);

            const std::string preludeSource(prelude);
            const std::string vertexSource = preludeSource + vert;
            const std::string fragmentSource = preludeSource + frag;

            auto& context = static_cast<Context&>(gfxContext);
            shader = context.createProgram(
                ShaderID, shaderName, vertexSource, fragmentSource, programParameters, additionalDefines);
            assert(shader);
            if (!shader || !registerShader(shader, shaderName)) {
                assert(false);
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }

            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Vulkan>;
            for (const auto& attrib : ShaderClass::attributes) {
                if (!propertiesAsUniforms.second.count(attrib.id)) {
                    shader->initAttribute(attrib);
                }
            }
            for (const auto& attrib : ShaderClass::instanceAttributes) {
                shader->initInstanceAttribute(attrib);
            }
            for (const auto& texture : ShaderClass::textures) {
                shader->initTexture(texture);
            }
        }
        return shader;
    }
};

} // namespace vulkan
} // namespace mbgl
