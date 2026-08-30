#include <mln/plugin/plugin_shader.hpp>

#include <mln/gfx/backend.hpp>
#include <mln/gfx/gfx_types.hpp>
#include <mln/gfx/shader_group.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/util/logging.hpp>

#if MLN_RENDER_BACKEND_OPENGL
#include <mln/gl/context.hpp>
#include <mln/shaders/gl/shader_info.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#elif MLN_RENDER_BACKEND_VULKAN
#include <mln/vulkan/context.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>
#elif MLN_RENDER_BACKEND_METAL
#include <mln/mtl/context.hpp>
#include <mln/shaders/mtl/common.hpp>
#include <mln/shaders/mtl/shader_program.hpp>
#endif

#include <stdexcept>
#include <sstream>

namespace mln {
namespace plugin {
namespace {

#if MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_METAL
gfx::AttributeDataType attributeType(mln_plugin_vertex_attribute_type type) {
    switch (type) {
        case MLN_PLUGIN_VERTEX_INT16:
            return gfx::AttributeDataType::Short;
        case MLN_PLUGIN_VERTEX_INT16_X2:
            return gfx::AttributeDataType::Short2;
        case MLN_PLUGIN_VERTEX_UINT16:
            return gfx::AttributeDataType::UShort;
        case MLN_PLUGIN_VERTEX_UINT16_X2:
            return gfx::AttributeDataType::UShort2;
        case MLN_PLUGIN_VERTEX_FLOAT:
            return gfx::AttributeDataType::Float;
        case MLN_PLUGIN_VERTEX_FLOAT_X2:
            return gfx::AttributeDataType::Float2;
        case MLN_PLUGIN_VERTEX_FLOAT_X3:
            return gfx::AttributeDataType::Float3;
        case MLN_PLUGIN_VERTEX_FLOAT_X4:
            return gfx::AttributeDataType::Float4;
        case MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED:
            return gfx::AttributeDataType::UByte4;
    }
    return gfx::AttributeDataType::Invalid;
}
#endif

#if MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_METAL
const ShaderSource* findSource(const ShaderDefinition& shader, mln_plugin_backend backend) {
    for (const auto& source : shader.sources) {
        if (source.backend == backend) return &source;
    }
    return nullptr;
}
#endif

std::string resourcePrelude(const ShaderDefinition& shader) {
    std::ostringstream output;
    for (const auto& uniform : shader.uniformBlocks) {
        auto bindingID = uniform.bindingID;
#if MLN_RENDER_BACKEND_VULKAN
        // Uniform arrays use MapLibre's global IDs, while Vulkan descriptor
        // bindings are local to their descriptor set. Built-in shaders perform
        // this same mapping through their backend-specific prelude constants.
        bindingID -= uniform.scope == MLN_PLUGIN_UNIFORM_SCOPE_LAYER ? shaders::layerSSBOStartId
                                                                     : shaders::drawableSSBOStartId;
#endif
        output << "#define MLN_PLUGIN_UNIFORM_" << uniform.id << "_BINDING " << bindingID << '\n';
    }
    for (const auto& texture : shader.textures) {
        output << "#define MLN_PLUGIN_TEXTURE_" << texture.id << "_BINDING " << texture.location << '\n';
    }
    return output.str();
}

class PluginShaderGroup final : public gfx::ShaderGroup {
public:
    PluginShaderGroup(ShaderDefinition definition_, ProgramParameters parameters_)
        : definition(std::move(definition_)),
          parameters(std::move(parameters_)) {}

    gfx::ShaderPtr getOrCreateShader(gfx::Context& context, const StringIDSetsPair&, std::string_view) override {
        const auto name = shaderGroupName(definition.pluginID, definition.id);
        if (auto existing = getShader(name)) return existing;
        const auto pluginPrelude = resourcePrelude(definition);

        gfx::ShaderPtr shader;
#if MLN_RENDER_BACKEND_OPENGL
        const auto* source = findSource(definition, MLN_PLUGIN_BACKEND_OPENGL);
        if (!source) return {};
        std::vector<shaders::AttributeInfo> attributes;
        attributes.reserve(definition.attributes.size());
        for (const auto& attr : definition.attributes) attributes.emplace_back(attr.name, attr.id);
        std::vector<shaders::UniformBlockInfo> uniformBlocks;
        uniformBlocks.reserve(definition.uniformBlocks.size());
        for (const auto& uniform : definition.uniformBlocks) {
            uniformBlocks.emplace_back(uniform.name, uniform.bindingID);
        }
        std::vector<shaders::TextureInfo> textures;
        textures.reserve(definition.textures.size());
        for (const auto& texture : definition.textures) {
            textures.emplace_back(texture.name, texture.id);
        }
        shader = gl::ShaderProgramGL::create(static_cast<gl::Context&>(context),
                                             parameters.withProgramType(shaders::BuiltIn::None),
                                             definition.attributes.front().name,
                                             uniformBlocks,
                                             textures,
                                             attributes,
                                             pluginPrelude + source->vertex,
                                             pluginPrelude + source->fragment);
#elif MLN_RENDER_BACKEND_VULKAN
        const auto* source = findSource(definition, MLN_PLUGIN_BACKEND_VULKAN);
        if (!source) return {};
        auto created = static_cast<vulkan::Context&>(context).createProgram(shaders::BuiltIn::None,
                                                                            name,
                                                                            pluginPrelude + source->vertex,
                                                                            pluginPrelude + source->fragment,
                                                                            parameters,
                                                                            {});
        if (!created) return {};
        auto typed = std::shared_ptr<vulkan::ShaderProgram>(std::move(created));
        for (const auto& attr : definition.attributes) {
            typed->initVertexAttribute({attr.location, attributeType(attr.type), attr.id});
        }
        for (const auto& texture : definition.textures) {
            typed->initTexture({texture.location, texture.id});
        }
        shader = std::move(typed);
#elif MLN_RENDER_BACKEND_METAL
        const auto* source = findSource(definition, MLN_PLUGIN_BACKEND_METAL);
        if (!source) return {};
        auto created = static_cast<mtl::Context&>(context).createProgram(
            shaders::BuiltIn::None,
            name,
            std::string(shaders::prelude) + pluginPrelude + source->vertex,
            source->vertexEntryPoint,
            source->fragmentEntryPoint,
            parameters,
            {});
        if (!created) return {};
        auto typed = std::shared_ptr<mtl::ShaderProgram>(std::move(created));
        for (const auto& attr : definition.attributes) {
            typed->initVertexAttribute(
                {attr.location, attributeType(attr.type), shaders::maxUBOCountPerShader + attr.location, attr.id});
        }
        for (const auto& texture : definition.textures) {
            typed->initTexture({texture.location, texture.id});
        }
        shader = std::move(typed);
#else
        (void)context;
#endif
        if (!shader) return {};
        if (!registerShader(gfx::ShaderPtr(shader), name)) {
            return getShader(name);
        }
        return shader;
    }

private:
    ShaderDefinition definition;
    ProgramParameters parameters;
};

} // namespace

std::string shaderGroupName(const std::string& pluginID, const std::string& shaderID) {
    return "plugin/" + pluginID + "/" + shaderID;
}

void registerPluginShaderGroups(gfx::ShaderRegistry& registry, const ProgramParameters& parameters) {
    for (const auto& layerType : PluginRegistry::get().allLayerTypes()) {
        for (const auto& shader : layerType.shaders) {
            const auto name = shaderGroupName(shader.pluginID, shader.id);
            if (registry.isShaderGroup(name)) continue;
            if (!registry.registerShaderGroup(std::make_shared<PluginShaderGroup>(shader, parameters), name)) {
                throw std::runtime_error("Failed to register plugin shader group '" + name + "'");
            }
        }
    }
}

} // namespace plugin
} // namespace mln
