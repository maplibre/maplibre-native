#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/common.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <fstream>

namespace mbgl {
namespace webgpu {

namespace {
WGPUBlendOperation webgpuBlendOperation(const gfx::ColorBlendEquationType& equation) {
    switch (equation) {
        case gfx::ColorBlendEquationType::Add:
            return WGPUBlendOperation_Add;
        case gfx::ColorBlendEquationType::Subtract:
            return WGPUBlendOperation_Subtract;
        case gfx::ColorBlendEquationType::ReverseSubtract:
            return WGPUBlendOperation_ReverseSubtract;
        default:
            return WGPUBlendOperation_Add;
    }
}

WGPUBlendFactor webgpuBlendFactor(const gfx::ColorBlendFactorType& factor) {
    switch (factor) {
        case gfx::ColorBlendFactorType::Zero:
            return WGPUBlendFactor_Zero;
        case gfx::ColorBlendFactorType::One:
            return WGPUBlendFactor_One;
        case gfx::ColorBlendFactorType::SrcColor:
            return WGPUBlendFactor_Src;
        case gfx::ColorBlendFactorType::OneMinusSrcColor:
            return WGPUBlendFactor_OneMinusSrc;
        case gfx::ColorBlendFactorType::SrcAlpha:
            return WGPUBlendFactor_SrcAlpha;
        case gfx::ColorBlendFactorType::OneMinusSrcAlpha:
            return WGPUBlendFactor_OneMinusSrcAlpha;
        case gfx::ColorBlendFactorType::DstAlpha:
            return WGPUBlendFactor_DstAlpha;
        case gfx::ColorBlendFactorType::OneMinusDstAlpha:
            return WGPUBlendFactor_OneMinusDstAlpha;
        case gfx::ColorBlendFactorType::DstColor:
            return WGPUBlendFactor_Dst;
        case gfx::ColorBlendFactorType::OneMinusDstColor:
            return WGPUBlendFactor_OneMinusDst;
        case gfx::ColorBlendFactorType::SrcAlphaSaturate:
            return WGPUBlendFactor_SrcAlphaSaturated;
        case gfx::ColorBlendFactorType::ConstantColor:
            return WGPUBlendFactor_Constant;
        case gfx::ColorBlendFactorType::OneMinusConstantColor:
            return WGPUBlendFactor_OneMinusConstant;
        case gfx::ColorBlendFactorType::ConstantAlpha:
            return WGPUBlendFactor_Constant;
        case gfx::ColorBlendFactorType::OneMinusConstantAlpha:
            return WGPUBlendFactor_OneMinusConstant;
        default:
            return WGPUBlendFactor_One;
    }
}
} // namespace

ShaderProgram::ShaderProgram(std::string name,
                           RendererBackend& backend_,
                           WGPUShaderModule vertexModule,
                           WGPUShaderModule fragmentModule)
    : ShaderProgramBase(),
      shaderName(std::move(name)),
      backend(backend_),
      vertexShaderModule(vertexModule),
      fragmentShaderModule(fragmentModule) {
    createPipelineLayout();
}

// Minimal constructor for Context::createShader compatibility
ShaderProgram::ShaderProgram(Context& context,
                           const std::string& vertexSource,
                           const std::string& fragmentSource)
    : ShaderProgramBase(),
      shaderName("ContextShader"),
      backend(static_cast<RendererBackend&>(context.getBackend())) {

    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (!device) return;

    // Get the prelude from common.hpp
    using PreludeShader = shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::WebGPU>;
    std::string vertexWithPrelude = std::string(PreludeShader::prelude) + "\n" + vertexSource;
    std::string fragmentWithPrelude = std::string(PreludeShader::prelude) + "\n" + fragmentSource;

    // Don't log here since shaderName will be updated by the template constructor
    // and the files would be misleading

    // Create vertex shader module
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = (WGPUSType)0x00040006;  // WGPUSType_ShaderModuleWGSLDescriptor
    wgslDesc.chain.next = nullptr;
    WGPUStringView vertexCode = {vertexWithPrelude.c_str(), vertexWithPrelude.length()};
    wgslDesc.code = vertexCode;

    WGPUShaderModuleDescriptor vertexShaderDesc = {};
    WGPUStringView vertexLabel = {"Vertex Shader Module", strlen("Vertex Shader Module")};
    vertexShaderDesc.label = vertexLabel;
    vertexShaderDesc.nextInChain = (WGPUChainedStruct*)&wgslDesc;

    vertexShaderModule = wgpuDeviceCreateShaderModule(device, &vertexShaderDesc);
    if (!vertexShaderModule) {
        Log::Error(Event::Render, "Failed to create vertex shader module for " + shaderName);
        Log::Error(Event::Render, "Vertex shader source length: " + std::to_string(vertexWithPrelude.length()));
    } else {
        Log::Info(Event::Render, "Successfully created vertex shader module for " + shaderName);
    }

    // Create fragment shader module
    WGPUStringView fragmentCode = {fragmentWithPrelude.c_str(), fragmentWithPrelude.length()};
    wgslDesc.code = fragmentCode;

    WGPUShaderModuleDescriptor fragmentShaderDesc = {};
    WGPUStringView fragmentLabel = {"Fragment Shader Module", strlen("Fragment Shader Module")};
    fragmentShaderDesc.label = fragmentLabel;
    fragmentShaderDesc.nextInChain = (WGPUChainedStruct*)&wgslDesc;

    fragmentShaderModule = wgpuDeviceCreateShaderModule(device, &fragmentShaderDesc);
    if (!fragmentShaderModule) {
        Log::Error(Event::Render, "Failed to create fragment shader module for " + shaderName);
        Log::Error(Event::Render, "Fragment shader source length: " + std::to_string(fragmentWithPrelude.length()));
    } else {
        Log::Info(Event::Render, "Successfully created fragment shader module for " + shaderName);
    }

    createPipelineLayout();
}

ShaderProgram::~ShaderProgram() {
    // Release cached pipelines
    for (auto& pair : renderPipelineCache) {
        if (pair.second) {
            wgpuRenderPipelineRelease(pair.second);
        }
    }
    renderPipelineCache.clear();

    // Release resources
    if (pipelineLayout) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        pipelineLayout = nullptr;
    }

    if (bindGroupLayout) {
        wgpuBindGroupLayoutRelease(bindGroupLayout);
        bindGroupLayout = nullptr;
    }

    // Release shader modules if we created them
    if (vertexShaderModule) {
        wgpuShaderModuleRelease(vertexShaderModule);
        vertexShaderModule = nullptr;
    }

    if (fragmentShaderModule) {
        wgpuShaderModuleRelease(fragmentShaderModule);
        fragmentShaderModule = nullptr;
    }
}

WGPURenderPipeline ShaderProgram::getRenderPipeline(const gfx::Renderable& renderable,
                                                   const WGPUVertexBufferLayout* vertexLayouts,
                                                   uint32_t vertexLayoutCount,
                                                   const gfx::ColorMode& colorMode,
                                                   const std::optional<std::size_t> reuseHash) {
    // Check cache first
    if (reuseHash.has_value()) {
        auto it = renderPipelineCache.find(reuseHash.value());
        if (it != renderPipelineCache.end()) {
            return it->second;
        }
    }

    // Create new pipeline
    WGPURenderPipeline pipeline = createPipeline(vertexLayouts, vertexLayoutCount, colorMode);

    // Cache the pipeline if we have a reuse hash
    if (reuseHash.has_value() && pipeline) {
        renderPipelineCache[reuseHash.value()] = pipeline;
    }

    return pipeline;
}

std::optional<size_t> ShaderProgram::getSamplerLocation(const size_t id) const {
    return (id < textureBindings.size()) ? textureBindings[id] : std::nullopt;
}

void ShaderProgram::initAttribute(const shaders::AttributeInfo& info) {
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must be unique, if there's a conflict check the `attributes` array in the shader
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) {
        assert(attrib.getIndex() != index);
    });
    instanceAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) {
        assert(attrib.getIndex() != index);
    });
#endif
    vertexAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initInstanceAttribute(const shaders::AttributeInfo& info) {
    // Index is the block index of the instance attribute
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must not be reused by regular attributes or uniform blocks
    // More than one instance attribute can have the same index, if they share the block
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) {
        assert(attrib.getIndex() != index);
    });
#endif
    instanceAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initTexture(const shaders::TextureInfo& info) {
    assert(info.id < textureBindings.size());
    if (info.id >= textureBindings.size()) {
        return;
    }
    textureBindings[info.id] = info.index;
}

void ShaderProgram::createPipelineLayout() {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) return;

    // Create bind group layout for uniforms
    std::vector<WGPUBindGroupLayoutEntry> bindingEntries;

    // Create bindings 0-5 for all possible uniform buffer indices
    for (uint32_t binding = 0; binding <= 5; ++binding) {
        WGPUBindGroupLayoutEntry entry = {};
        entry.binding = binding;
        entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
        entry.buffer.type = WGPUBufferBindingType_Uniform;
        entry.buffer.hasDynamicOffset = 0;
        entry.buffer.minBindingSize = 0;
        bindingEntries.push_back(entry);
    }

    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    WGPUStringView bindGroupLabel = {"Uniform Bind Group Layout", strlen("Uniform Bind Group Layout")};
    bindGroupLayoutDesc.label = bindGroupLabel;
    bindGroupLayoutDesc.entryCount = bindingEntries.size();
    bindGroupLayoutDesc.entries = bindingEntries.data();

    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);

    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    WGPUStringView pipelineLayoutLabel = {"Pipeline Layout", strlen("Pipeline Layout")};
    pipelineLayoutDesc.label = pipelineLayoutLabel;

    if (bindGroupLayout) {
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    } else {
        pipelineLayoutDesc.bindGroupLayoutCount = 0;
        pipelineLayoutDesc.bindGroupLayouts = nullptr;
    }

    pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);
}

WGPURenderPipeline ShaderProgram::createPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                                uint32_t vertexLayoutCount,
                                                const gfx::ColorMode& colorMode) {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device || !vertexShaderModule || !fragmentShaderModule || !pipelineLayout) {
        Log::Error(Event::Render, "createPipeline missing requirement: device=" +
            std::to_string(device != nullptr) + ", vertexModule=" +
            std::to_string(vertexShaderModule != nullptr) + ", fragmentModule=" +
            std::to_string(fragmentShaderModule != nullptr) + ", pipelineLayout=" +
            std::to_string(pipelineLayout != nullptr));
        return nullptr;
    }

    // Set up vertex state
    WGPUVertexState vertexState = {};
    vertexState.module = vertexShaderModule;
    WGPUStringView vertexEntryPoint = {"main", strlen("main")};
    vertexState.entryPoint = vertexEntryPoint;
    vertexState.bufferCount = vertexLayoutCount;
    vertexState.buffers = vertexLayouts;

    // Set up fragment state with color mode blending
    WGPUBlendComponent alphaBlend = {};
    alphaBlend.operation = webgpuBlendOperation(gfx::ColorBlendEquationType::Add);
    alphaBlend.srcFactor = webgpuBlendFactor(gfx::ColorBlendFactorType::SrcAlpha);
    alphaBlend.dstFactor = webgpuBlendFactor(gfx::ColorBlendFactorType::OneMinusSrcAlpha);

    WGPUBlendComponent colorBlend = {};
    colorBlend.operation = webgpuBlendOperation(gfx::ColorBlendEquationType::Add);
    colorBlend.srcFactor = webgpuBlendFactor(gfx::ColorBlendFactorType::SrcAlpha);
    colorBlend.dstFactor = webgpuBlendFactor(gfx::ColorBlendFactorType::OneMinusSrcAlpha);

    // Apply color mode blending if not replace
    if (!colorMode.blendFunction.is<gfx::ColorMode::Replace>()) {
        colorMode.blendFunction.match(
            [&](const auto& blendFunction) {
                colorBlend.operation = webgpuBlendOperation(gfx::ColorBlendEquationType(blendFunction.equation));
                colorBlend.srcFactor = webgpuBlendFactor(gfx::ColorBlendFactorType(blendFunction.srcFactor));
                colorBlend.dstFactor = webgpuBlendFactor(gfx::ColorBlendFactorType(blendFunction.dstFactor));
                alphaBlend = colorBlend;
            });
    }

    WGPUBlendState blendState = {};
    blendState.color = colorBlend;
    blendState.alpha = alphaBlend;

    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = (colorMode.mask.r ? WGPUColorWriteMask_Red : WGPUColorWriteMask_None) |
                           (colorMode.mask.g ? WGPUColorWriteMask_Green : WGPUColorWriteMask_None) |
                           (colorMode.mask.b ? WGPUColorWriteMask_Blue : WGPUColorWriteMask_None) |
                           (colorMode.mask.a ? WGPUColorWriteMask_Alpha : WGPUColorWriteMask_None);

    WGPUFragmentState fragmentState = {};
    fragmentState.module = fragmentShaderModule;
    WGPUStringView fragmentEntryPoint = {"main", strlen("main")};
    fragmentState.entryPoint = fragmentEntryPoint;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // Set up primitive state
    WGPUPrimitiveState primitiveState = {};
    primitiveState.topology = WGPUPrimitiveTopology_TriangleList;
    primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitiveState.frontFace = WGPUFrontFace_CCW;
    primitiveState.cullMode = WGPUCullMode_None;

    // Set up depth stencil state
    WGPUDepthStencilState depthStencilState = {};
    depthStencilState.format = WGPUTextureFormat_Depth24PlusStencil8;
    // Enable depth testing for fill layers - they need proper depth ordering
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
    depthStencilState.depthCompare = WGPUCompareFunction_LessEqual;
    depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
    depthStencilState.stencilFront.failOp = WGPUStencilOperation_Keep;
    depthStencilState.stencilFront.depthFailOp = WGPUStencilOperation_Keep;
    depthStencilState.stencilFront.passOp = WGPUStencilOperation_Keep;
    depthStencilState.stencilBack = depthStencilState.stencilFront;
    depthStencilState.stencilReadMask = 0xFF;
    depthStencilState.stencilWriteMask = 0xFF;

    // Create render pipeline
    WGPURenderPipelineDescriptor pipelineDesc = {};
    WGPUStringView pipelineLabel = {shaderName.c_str(), shaderName.length()};
    pipelineDesc.label = pipelineLabel;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitiveState;
    pipelineDesc.depthStencil = &depthStencilState;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = 0;

    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
    if (!pipeline) {
        Log::Error(Event::Render, shaderName + " pipeline creation failed");
    }
    return pipeline;
}

} // namespace webgpu
} // namespace mbgl