#pragma once

#include <mln/gfx/shader_group.hpp>
#include <mln/mtl/context.hpp>
#include <mln/shaders/feature_index.hpp>
#include <mln/shaders/mtl/common.hpp>
#include <mln/shaders/mtl/shader_program.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/util/hash.hpp>
#include <mln/util/containers.hpp>

#include <string>

namespace mln::mtl {

class ShaderGroupBase : public gfx::ShaderGroup {
protected:
    ShaderGroupBase(const ProgramParameters& parameters_)
        : programParameters(parameters_) {}

    using DefinesMap = mln::unordered_map<std::string, std::string>;
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
        using ShaderSource = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>;
        constexpr auto& name = ShaderSource::name;
        constexpr auto& prelude = ShaderSource::prelude;
        constexpr auto& source = ShaderSource::source;
        constexpr auto& vertMain = ShaderSource::vertexMainFunction;
        constexpr auto& fragMain = ShaderSource::fragmentMainFunction;

        std::size_t seed = 0;
        mln::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
        mln::util::hash_combine(seed, programParameters.getDefinesHash());
        const std::string shaderName = getShaderName(name, seed);

        auto shader = get<mtl::ShaderProgram>(shaderName);
        if (!shader) {
            DefinesMap additionalDefines;
            addAdditionalDefines(propertiesAsUniforms, additionalDefines);
            constexpr auto featureIndexSlot = shaders::featureIndexAttributeId<ShaderID>();
            [[maybe_unused]] bool useFeatureIndex = false;
            if constexpr (featureIndexSlot.has_value()) {
                constexpr auto attributes = shaders::featureAttributes<ShaderID>();
                useFeatureIndex = defineFeatureIndex(attributes, propertiesAsUniforms, additionalDefines);
                if (useFeatureIndex) {
                    shaders::defineFeatureConstants(shaders::featureLayout(attributes, propertiesAsUniforms.second),
                                                    additionalDefines);
                }
            }

            auto& context = static_cast<Context&>(gfxContext);
            const auto shaderSource = std::string(shaders::prelude) + prelude + source;
            shader = context.createProgram(
                ShaderID, shaderName, shaderSource, vertMain, fragMain, programParameters, additionalDefines);
            assert(shader);
            if (!shader || !registerShader(shader, shaderName)) {
                assert(false);
                Log::Error(Event::Shader, "Failed to register " + shaderName + " with shader group!");
                return nullptr;
            }

            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>;
            for (const auto& attrib : ShaderClass::attributes) {
                // The feature-index slot reuses a paint attribute id. Membership of that
                // id in the uniform set means the property is constant, not that the slot
                // is absent. Bind the slot only when this variant reads the feature table.
                if constexpr (featureIndexSlot.has_value()) {
                    if (attrib.id == *featureIndexSlot) {
                        if (useFeatureIndex) {
                            shader->initVertexAttribute(attrib);
                        }
                        continue;
                    }
                }
                if (!propertiesAsUniforms.second.contains(attrib.id)) {
                    shader->initVertexAttribute(attrib);
                }
            }
            for (const auto& attrib : ShaderClass::instanceAttributes) {
                if (!propertiesAsUniforms.second.count(attrib.id)) {
                    shader->initInstanceAttribute(attrib);
                }
            }
            for (const auto& texture : ShaderClass::textures) {
                shader->initTexture(texture);
            }
        }
        return shader;
    }
};

} // namespace mln::mtl
