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
#include <glslang/SPIRV/GlslangToSpv.h>

using namespace std::string_literals;

namespace mbgl {

namespace vulkan {

ShaderProgram::ShaderProgram(const std::string& name,
                             const std::string_view& vertex,
                             const std::string_view& fragment,
                             const ProgramParameters& programParameters,
                             const mbgl::unordered_map<std::string, std::string>& additionalDefines,
                             RendererBackend& backend_)
    : ShaderProgramBase(),
      shaderName(name),
      backend(backend_) {

    const char* const shaderNameRaw = shaderName.c_str();

    constexpr auto targetClientVersion = glslang::EShTargetVulkan_1_0;
    constexpr auto targetLanguageVersion = glslang::EShTargetSpv_1_0;
    constexpr auto defaultVersion = 450;
    constexpr auto messages = EShMsgSpvRules | EShMsgVulkanRules;
    const auto defaultResources = GetDefaultResources();

    std::string defineStr;
    for (const auto& define : additionalDefines) {
        defineStr += "#define " + define.first + " " + define.second + "\n";
    }

    const auto compileGlsl = [&](const EShLanguage& language, const std::string_view& data, const char* prelude) -> std::vector<unsigned int> {
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
            return {};
        }

        glslang::TProgram glslProgram;
        glslProgram.addShader(&glslShader);

        if (!glslProgram.link(messages)) {
            mbgl::Log::Error(mbgl::Event::Shader, shaderName + " - " + glslProgram.getInfoLog());
            return {};
        }

        const auto intermediate = glslProgram.getIntermediate(language);

        std::vector<unsigned int> spirv;
        glslang::GlslangToSpv(*intermediate, spirv);

        return spirv;
    };

    const auto& vertexSpirv = compileGlsl(EShLanguage::EShLangVertex, vertex,
        shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::Vulkan>::vertex);
    const auto& fragmentSpirv = compileGlsl(EShLanguage::EShLangFragment, fragment,
        shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::Vulkan>::fragment);

    if (vertexSpirv.empty() || fragmentSpirv.empty())
        return;

    const auto& device = backend.getDevice();

    vertexShader = device->createShaderModuleUnique(
        vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), vertexSpirv));
    fragmentShader = device->createShaderModuleUnique(
        vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), fragmentSpirv));

    backend.setDebugName(vertexShader.get(), shaderName + ".vert");
    backend.setDebugName(fragmentShader.get(), shaderName + ".frag");
}

ShaderProgram::~ShaderProgram() noexcept = default;

const std::vector<vk::DescriptorSetLayout>& ShaderProgram::getDescriptorSetLayouts() {
    if (!descriptorSetLayouts.empty())
        return descriptorSetLayouts;

    const auto& dummy = static_cast<Context*>(&backend.getContext())->getDummyDescriptorSetLayout();

    // descriptor set layout
    uniformBlocks.visit([&](const gfx::UniformBlock& block_) {
        const auto block = static_cast<const UniformBlock*>(&block_);

        // fill descriptor gap
        // if we use sets 1 and 4 this will generate dummy layouts for sets 0/2/3
        for (size_t i = 0, gap = block->getIndex() - descriptorSetLayouts.size(); i < gap; ++i)
            descriptorSetLayouts.push_back(dummy.get());

        vk::ShaderStageFlags stages;
        
        if (block->getBindVertex())
            stages |= vk::ShaderStageFlagBits::eVertex;

        if (block->getBindFragment()) 
            stages |= vk::ShaderStageFlagBits::eFragment;

        const auto descriptorSetBinding = vk::DescriptorSetLayoutBinding()
            .setBinding(0)
            .setStageFlags(stages)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1);

        const auto& descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindings(descriptorSetBinding);

        usedDescriptorSetLayouts.push_back(
            backend.getDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo));

        const auto& layout = usedDescriptorSetLayouts.back().get();

        backend.setDebugName(layout, shaderName + "_descriptorSetLayout_" + std::to_string(block->getIndex()));

        descriptorSetLayouts.push_back(layout);
    });

    // TODO add texture descriptors

    return descriptorSetLayouts;
}

const vk::UniquePipelineLayout& ShaderProgram::getPipelineLayout() {
    if (pipelineLayout) 
        return pipelineLayout;

    const auto& descriptorSetLayouts = getDescriptorSetLayouts();

    pipelineLayout = backend.getDevice()->createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo().setSetLayouts(descriptorSetLayouts)
    );

    backend.setDebugName(pipelineLayout.get(), shaderName + "_pipelineLayout");

    return pipelineLayout;
}

const vk::UniquePipeline& ShaderProgram::getPipeline(const PipelineInfo& pipelineInfo) {
    auto& pipeline = pipelines[pipelineInfo.hash()];
    if (pipeline) 
        return pipeline;

    // input layout
    std::vector<vk::VertexInputBindingDescription> inputBindings;
    std::vector<vk::VertexInputAttributeDescription> inputAttributes;

    if (vertexAttributes.getMaxCount() > 0) {
        inputBindings.push_back(vk::VertexInputBindingDescription()
            .setBinding(0)
            .setStride(vertexAttributes.getTotalSize())
            .setInputRate(vk::VertexInputRate::eVertex)
        );
    }

    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { 
        inputAttributes.push_back(vk::VertexInputAttributeDescription()
            .setBinding(0)
            .setLocation(attrib.getIndex())
            .setFormat(PipelineInfo::vulkanFormat(attrib.getDataType()))
            .setOffset(attrib.getSharedVertexOffset())
        );
    });

    if (instanceAttributes.getMaxCount() > 0) {
        inputBindings.push_back(vk::VertexInputBindingDescription()
            .setBinding(1)
            .setStride(instanceAttributes.getTotalSize())
            .setInputRate(vk::VertexInputRate::eInstance)
        );
    }

    instanceAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { 
        inputAttributes.push_back(vk::VertexInputAttributeDescription()
            .setBinding(1)
            .setLocation(attrib.getIndex())
            .setFormat(PipelineInfo::vulkanFormat(attrib.getDataType()))
            .setOffset(attrib.getSharedVertexOffset())
        );
    });

    const auto& vertexInputState = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptions(inputBindings)
        .setVertexAttributeDescriptions(inputAttributes);

    const auto& inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(pipelineInfo.topology);

    const auto& renderableResource = backend.getDefaultRenderable().getResource<RenderableResource>();
    const auto& renderableExtent = renderableResource.getExtent();
    
    const vk::Viewport viewportExtent(0.0f, 0.0f, renderableExtent.width, renderableExtent.height, 0.0f, 1.0f);
    const vk::Rect2D scissorRect({}, { renderableExtent.width, renderableExtent.height });

    const auto& viewportState = vk::PipelineViewportStateCreateInfo()
        .setViewportCount(1)
        .setPViewports(&viewportExtent)
        .setScissorCount(1)
        .setPScissors(&scissorRect);

    const auto rasterState = vk::PipelineRasterizationStateCreateInfo()
        .setCullMode(pipelineInfo.cullMode)
        .setFrontFace(pipelineInfo.frontFace)
        .setPolygonMode(pipelineInfo.polygonMode)
        .setLineWidth(1.0f);

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
        .setDepthTestEnable(pipelineInfo.depthTest)
        .setDepthWriteEnable(pipelineInfo.depthWrite)
        .setDepthBoundsTestEnable(false)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setDepthCompareOp(pipelineInfo.depthFunction)
        .setStencilTestEnable(false)
        ;

    const auto& colorBlendAttachments = vk::PipelineColorBlendAttachmentState()
        .setBlendEnable(pipelineInfo.colorBlend)
        .setColorBlendOp(pipelineInfo.colorBlendFunction)
        .setSrcColorBlendFactor(pipelineInfo.srcBlendFactor)
        .setDstColorBlendFactor(pipelineInfo.dstBlendFactor)
        .setAlphaBlendOp(pipelineInfo.colorBlendFunction)
        .setSrcAlphaBlendFactor(pipelineInfo.srcBlendFactor)
        .setDstAlphaBlendFactor(pipelineInfo.dstBlendFactor)
        .setColorWriteMask(pipelineInfo.colorMask);

    const auto& colorBlendState = vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&colorBlendAttachments)
        .setLogicOpEnable(VK_FALSE)
        .setLogicOp(vk::LogicOp::eCopy);

    std::vector<vk::DynamicState> dynamicValues = {
        // values available for core 1.0
        //vk::DynamicState::eViewport,
        //vk::DynamicState::eScissor,
        //vk::DynamicState::eLineWidth,
        //vk::DynamicState::eStencilCompareMask,
        //vk::DynamicState::eStencilWriteMask,
        //vk::DynamicState::eStencilReference,
        //vk::DynamicState::eBlendConstants,
        //vk::DynamicState::eDepthBias,
        //vk::DynamicState::eDepthBounds,
    };

    if (pipelineInfo.usesBlendConstants()) {
        dynamicValues.push_back(vk::DynamicState::eBlendConstants);
    }

    if (pipelineInfo.wideLines) {
        dynamicValues.push_back(vk::DynamicState::eLineWidth);
    }

    const vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicValues);

    const auto& device = backend.getDevice();
    const auto& pipelineLayout = getPipelineLayout();

    const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vertexShader.get())
            .setPName("main"),

        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragmentShader.get())
            .setPName("main")
    };

    const auto& pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
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
        .setRenderPass(renderableResource.getRenderPass().get());

    pipeline = std::move(device->createGraphicsPipelineUnique(nullptr, pipelineCreateInfo).value);
    backend.setDebugName(pipeline.get(), shaderName + "_pipeline");

    return pipeline;
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
