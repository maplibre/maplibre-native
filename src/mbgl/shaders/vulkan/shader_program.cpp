#include <mbgl/shaders/vulkan/shader_program.hpp>

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/uniform_block.hpp>
#include <mbgl/vulkan/uniform_buffer.hpp>
#include <mbgl/vulkan/vertex_attribute.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/shaders/vulkan/common.hpp>
#include <mbgl/util/logging.hpp>

#include <cstring>
#include <utility>
#include <algorithm>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

using namespace std::string_literals;

namespace mbgl {

namespace vulkan {

ShaderProgram::ShaderProgram(shaders::BuiltIn shaderID,
                             const std::string& name,
                             const std::string_view& vertex,
                             const std::string_view& fragment,
                             const ProgramParameters& programParameters,
                             const mbgl::unordered_map<std::string, std::string>& additionalDefines,
                             RendererBackend& backend_,
                             gfx::ContextObserver& observer)
    : ShaderProgramBase(),
      shaderName(name),
      backend(backend_) {
    std::string defineStr = programParameters.getDefinesString() + "\n\n";
    for (const auto& define : additionalDefines) {
        defineStr += "#define " + define.first + " " + define.second + "\n";
    }
    observer.onPreCompileShader(shaderID, gfx::Backend::Type::Metal, defineStr);

    constexpr auto targetClientVersion = glslang::EShTargetVulkan_1_0;
    constexpr auto targetLanguageVersion = glslang::EShTargetSpv_1_0;
    constexpr auto defaultVersion = 450;
    constexpr auto messages = EShMsgSpvRules | EShMsgVulkanRules;
    const auto defaultResources = GetDefaultResources();

    const auto compileGlsl = [&](const EShLanguage& language, const std::string_view& data, const char* prelude) {
        glslang::TShader glslShader(language);

        const auto preamble = defineStr + "\n" + prelude;
        const char* shaderData = data.data();
        const int shaderDataSize = data.size();

        glslShader.setPreamble(preamble.c_str());
        glslShader.setStringsWithLengths(&shaderData, &shaderDataSize, 1);
        glslShader.setEnvClient(glslang::EShClientVulkan, targetClientVersion);
        glslShader.setEnvTarget(glslang::EShTargetSpv, targetLanguageVersion);
        glslShader.setEntryPoint("main");

        if (!glslShader.parse(defaultResources, defaultVersion, ENoProfile, false, true, messages)) {
            mbgl::Log::Error(mbgl::Event::Shader, shaderName + " - " + glslShader.getInfoLog());
            observer.onShaderCompileFailed(shaderID, gfx::Backend::Type::Vulkan, defineStr);
            return std::vector<uint32_t>();
        }

        glslang::TProgram glslProgram;
        glslProgram.addShader(&glslShader);

        if (!glslProgram.link(messages)) {
            mbgl::Log::Error(mbgl::Event::Shader, shaderName + " - " + glslProgram.getInfoLog());
            observer.onShaderCompileFailed(shaderID, gfx::Backend::Type::Vulkan, defineStr);
            return std::vector<uint32_t>();
        }

        const auto intermediate = glslProgram.getIntermediate(language);

        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*intermediate, spirv);

        return spirv;
    };

    const auto& vertexSpirv = compileGlsl(
        EShLanguage::EShLangVertex,
        vertex,
        shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::Vulkan>::vertex);
    const auto& fragmentSpirv = compileGlsl(
        EShLanguage::EShLangFragment,
        fragment,
        shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::Vulkan>::fragment);

    if (vertexSpirv.empty() || fragmentSpirv.empty()) return;

    const auto& device = backend.getDevice();

    vertexShader = device->createShaderModuleUnique(
        vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), vertexSpirv));
    fragmentShader = device->createShaderModuleUnique(
        vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), fragmentSpirv));

    backend.setDebugName(vertexShader.get(), shaderName + ".vert");
    backend.setDebugName(fragmentShader.get(), shaderName + ".frag");

    observer.onPostCompileShader(shaderID, gfx::Backend::Type::Metal, defineStr);
}

ShaderProgram::~ShaderProgram() noexcept = default;

const vk::UniquePipeline& ShaderProgram::getPipeline(const PipelineInfo& pipelineInfo) {
    auto& pipeline = pipelines[pipelineInfo.hash()];
    if (pipeline) return pipeline;

    const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
                                      .setVertexBindingDescriptions(pipelineInfo.inputBindings)
                                      .setVertexAttributeDescriptions(pipelineInfo.inputAttributes);

    const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo().setTopology(pipelineInfo.topology);

    const auto renderableExtent = pipelineInfo.viewExtent;

    const vk::Viewport viewportExtent(0.0f, 0.0f, renderableExtent.width, renderableExtent.height, 0.0f, 1.0f);
    const vk::Rect2D scissorRect({}, {renderableExtent.width, renderableExtent.height});

    const auto viewportState = vk::PipelineViewportStateCreateInfo()
                                   .setViewportCount(1)
                                   .setPViewports(&viewportExtent)
                                   .setScissorCount(1)
                                   .setPScissors(&scissorRect);

    const auto rasterState = vk::PipelineRasterizationStateCreateInfo()
                                 .setCullMode(pipelineInfo.cullMode)
                                 .setFrontFace(pipelineInfo.frontFace)
                                 .setPolygonMode(pipelineInfo.polygonMode)
                                 .setLineWidth(1.0f);

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo().setRasterizationSamples(
        vk::SampleCountFlagBits::e1);

    const auto stencilState = vk::StencilOpState()
                                  .setCompareOp(pipelineInfo.stencilFunction)
                                  .setPassOp(pipelineInfo.stencilPass)
                                  .setFailOp(pipelineInfo.stencilFail)
                                  .setDepthFailOp(pipelineInfo.stencilDepthFail);

    const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
                                       .setDepthTestEnable(pipelineInfo.depthTest)
                                       .setDepthWriteEnable(pipelineInfo.depthWrite)
                                       .setDepthBoundsTestEnable(false)
                                       .setMinDepthBounds(0.0f)
                                       .setMaxDepthBounds(1.0f)
                                       .setDepthCompareOp(pipelineInfo.depthFunction)
                                       .setStencilTestEnable(pipelineInfo.stencilTest)
                                       .setFront(stencilState)
                                       .setBack(stencilState);

    const auto colorBlendAttachments = vk::PipelineColorBlendAttachmentState()
                                           .setBlendEnable(pipelineInfo.colorBlend)
                                           .setColorBlendOp(pipelineInfo.colorBlendFunction)
                                           .setSrcColorBlendFactor(pipelineInfo.srcBlendFactor)
                                           .setDstColorBlendFactor(pipelineInfo.dstBlendFactor)
                                           .setAlphaBlendOp(pipelineInfo.colorBlendFunction)
                                           .setSrcAlphaBlendFactor(pipelineInfo.srcBlendFactor)
                                           .setDstAlphaBlendFactor(pipelineInfo.dstBlendFactor)
                                           .setColorWriteMask(pipelineInfo.colorMask);

    const auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
                                     .setAttachmentCount(1)
                                     .setPAttachments(&colorBlendAttachments)
                                     .setLogicOpEnable(VK_FALSE)
                                     .setLogicOp(vk::LogicOp::eCopy);

    // values available for core 1.0
    // vk::DynamicState::eViewport,
    // vk::DynamicState::eScissor,
    // vk::DynamicState::eLineWidth,
    // vk::DynamicState::eStencilCompareMask,
    // vk::DynamicState::eStencilWriteMask,
    // vk::DynamicState::eStencilReference,
    // vk::DynamicState::eBlendConstants,
    // vk::DynamicState::eDepthBias,
    // vk::DynamicState::eDepthBounds,

    const auto& dynamicValues = pipelineInfo.getDynamicStates(backend);
    const vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicValues);

    const auto& device = backend.getDevice();
    auto& context = static_cast<Context&>(backend.getContext());
    const auto& pipelineLayout = pipelineInfo.usePushConstants ? context.getPushConstantPipelineLayout()
                                                               : context.getGeneralPipelineLayout();

    const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vertexShader.get())
            .setPName("main"),

        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragmentShader.get())
            .setPName("main")};

    const auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
                                        .setStages(shaderStages)
                                        .setPVertexInputState(&vertexInputState)
                                        .setPInputAssemblyState(&inputAssemblyState)
                                        .setPViewportState(&viewportState)
                                        .setPRasterizationState(&rasterState)
                                        .setPMultisampleState(&multisampleState)
                                        .setPDepthStencilState(&depthStencilState)
                                        .setPColorBlendState(&colorBlendState)
                                        .setPDynamicState(&dynamicState)
                                        .setLayout(pipelineLayout.get())
                                        .setRenderPass(pipelineInfo.renderPass);

    pipeline = std::move(device->createGraphicsPipelineUnique(nullptr, pipelineCreateInfo).value);
    backend.setDebugName(pipeline.get(), shaderName + "_pipeline");

    return pipeline;
}

bool ShaderProgram::hasTextures() const {
    return std::any_of(
        textureBindings.begin(), textureBindings.end(), [](const auto& texture) { return texture.has_value(); });
}

void ShaderProgram::initAttribute(const shaders::AttributeInfo& info) {
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must be unique, if there's a conflict check the `attributes` array in the shader
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
    instanceAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
    uniformBlocks.visit([&](const gfx::UniformBlock& block) { assert(block.getIndex() != index); });
#endif
    vertexAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initInstanceAttribute(const shaders::AttributeInfo& info) {
    // Index is the block index of the instance attribute
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must not be reused by regular attributes or uniform blocks
    // More than one instance attribute can have the same index, if they share the block
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
    uniformBlocks.visit([&](const gfx::UniformBlock& block) { assert(block.getIndex() != index); });
#endif
    instanceAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initUniformBlock(const shaders::UniformBlockInfo& info) {
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must be unique, if there's a conflict check the `attributes` array in the shader
    uniformBlocks.visit([&](const gfx::UniformBlock& block) { assert(block.getIndex() != index); });
#endif
    if (const auto& block_ = uniformBlocks.set(info.id, index, info.size)) {
        auto& block = static_cast<UniformBlock&>(*block_);
        block.setBindVertex(info.vertex);
        block.setBindFragment(info.fragment);
    }
}

void ShaderProgram::initTexture(const shaders::TextureInfo& info) {
    assert(info.id < textureBindings.size());
    if (info.id >= textureBindings.size()) {
        return;
    }
    textureBindings[info.id] = info.index;
}

} // namespace vulkan
} // namespace mbgl
