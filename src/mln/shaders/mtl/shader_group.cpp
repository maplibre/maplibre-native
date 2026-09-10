#include <mln/shaders/mtl/shader_group.hpp>
#include <mln/mtl/context.hpp>

namespace mln::mtl {

gfx::ShaderPtr ShaderGroup::getOrCreateShader(gfx::Context& gfxContext,
                                              const StringIDSetsPair& propertiesAsUniforms,
                                              std::string_view /*firstAttribName*/) {
    std::size_t seed = 0;
    mln::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
    mln::util::hash_combine(seed, programParameters.getDefinesHash());
    const std::string shaderName = getShaderName(info.name, seed);

    auto shader = get<mtl::ShaderProgram>(shaderName);
    if (!shader) {
        DefinesMap additionalDefines;
        addAdditionalDefines(propertiesAsUniforms, additionalDefines);

        auto& context = static_cast<Context&>(gfxContext);
        std::string shaderSource(shaders::prelude());
        shaderSource.append(info.prelude());
        shaderSource.append(info.source());
        shader = context.createProgram(info.id,
                                       shaderName,
                                       shaderSource,
                                       info.vertexMain,
                                       info.fragmentMain,
                                       programParameters,
                                       additionalDefines);
        assert(shader);
        if (!shader || !registerShader(shader, shaderName)) {
            assert(false);
            Log::Error(Event::Shader, "Failed to register " + shaderName + " with shader group!");
            return nullptr;
        }

        for (const auto& attrib : info.attributes) {
            if (!propertiesAsUniforms.second.count(attrib.id)) {
                shader->initVertexAttribute(attrib);
            }
        }
        for (const auto& attrib : info.instanceAttributes) {
            if (!propertiesAsUniforms.second.count(attrib.id)) {
                shader->initInstanceAttribute(attrib);
            }
        }
        for (const auto& texture : info.textures) {
            shader->initTexture(texture);
        }
    }
    return shader;
}

} // namespace mln::mtl
