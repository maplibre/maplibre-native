#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/common.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/gfx/color_mode.hpp>

namespace mbgl {
namespace webgpu {

// Metal-like constructor: takes pre-compiled shader modules
ShaderProgram::ShaderProgram(std::string name,
                           RendererBackend& backend_,
                           WGPUShaderModule vertexModule,
                           WGPUShaderModule fragmentModule)
    : shaderName(std::move(name)),
      backend(&backend_),
      vertexShaderModule(vertexModule),
      fragmentShaderModule(fragmentModule) {
    // Create bind group layout and pipeline layout (similar to Metal's approach)
    createPipelineLayout();
}

// Compatibility constructor
ShaderProgram::ShaderProgram(Context& context_,
                           const std::string& vertexSource,
                           const std::string& fragmentSource)
    : context(&context_) {
    createShaderModules(vertexSource, fragmentSource);
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

    // Release resources in reverse order of creation
    if (pipelineLayout) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        pipelineLayout = nullptr;
    }

    if (bindGroupLayout) {
        wgpuBindGroupLayoutRelease(bindGroupLayout);
        bindGroupLayout = nullptr;
    }

    // Only release shader modules if we created them (not if passed in)
    if (vertexShaderModule && context) {
        wgpuShaderModuleRelease(vertexShaderModule);
        vertexShaderModule = nullptr;
    }

    if (fragmentShaderModule && context) {
        wgpuShaderModuleRelease(fragmentShaderModule);
        fragmentShaderModule = nullptr;
    }
}

// Metal-like lazy pipeline creation with caching
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

// Metal-like sampler location getter
std::optional<size_t> ShaderProgram::getSamplerLocation(const size_t id) const {
    return (id < textureBindings.size()) ? textureBindings[id] : std::nullopt;
}

// Metal-like attribute initialization
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

// Metal-like instance attribute initialization
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

// Metal-like texture initialization
void ShaderProgram::initTexture(const shaders::TextureInfo& info) {
    assert(info.id < textureBindings.size());
    if (info.id >= textureBindings.size()) {
        return;
    }
    textureBindings[info.id] = info.index;
}

// Create shader modules from source (for compatibility constructors)
void ShaderProgram::createShaderModules(const std::string& vertexSource, const std::string& fragmentSource) {
    if (!context) return;

    auto& backendRef = static_cast<webgpu::RendererBackend&>(context->getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backendRef.getDevice());
    if (!device) return;

    // Get the prelude from common.hpp
    using PreludeShader = shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::WebGPU>;
    std::string vertexWithPrelude = std::string(PreludeShader::prelude) + "\n" + vertexSource;
    std::string fragmentWithPrelude = std::string(PreludeShader::prelude) + "\n" + fragmentSource;

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

    // Create fragment shader module
    WGPUStringView fragmentCode = {fragmentWithPrelude.c_str(), fragmentWithPrelude.length()};
    wgslDesc.code = fragmentCode;

    WGPUShaderModuleDescriptor fragmentShaderDesc = {};
    WGPUStringView fragmentLabel = {"Fragment Shader Module", strlen("Fragment Shader Module")};
    fragmentShaderDesc.label = fragmentLabel;
    fragmentShaderDesc.nextInChain = (WGPUChainedStruct*)&wgslDesc;

    fragmentShaderModule = wgpuDeviceCreateShaderModule(device, &fragmentShaderDesc);
}

// Create pipeline layout (similar to Metal's approach)
void ShaderProgram::createPipelineLayout() {
    WGPUDevice device = nullptr;
    if (backend) {
        device = static_cast<WGPUDevice>(backend->getDevice());
    } else if (context) {
        auto& backendRef = static_cast<webgpu::RendererBackend&>(context->getBackend());
        device = static_cast<WGPUDevice>(backendRef.getDevice());
    }

    if (!device) return;

    // Create bind group layout for uniforms (same as before)
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

// Create pipeline with given parameters (Metal-like lazy creation)
WGPURenderPipeline ShaderProgram::createPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                                uint32_t vertexLayoutCount,
                                                const gfx::ColorMode& colorMode) {
    WGPUDevice device = nullptr;
    if (backend) {
        device = static_cast<WGPUDevice>(backend->getDevice());
    } else if (context) {
        auto& backendRef = static_cast<webgpu::RendererBackend&>(context->getBackend());
        device = static_cast<WGPUDevice>(backendRef.getDevice());
    }

    if (!device || !vertexShaderModule || !fragmentShaderModule || !pipelineLayout) {
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
    alphaBlend.operation = getWGPUBlendOperation(gfx::ColorBlendEquationType::Add);
    alphaBlend.srcFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType::SrcAlpha);
    alphaBlend.dstFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType::OneMinusSrcAlpha);

    WGPUBlendComponent colorBlend = {};
    colorBlend.operation = getWGPUBlendOperation(gfx::ColorBlendEquationType::Add);
    colorBlend.srcFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType::SrcAlpha);
    colorBlend.dstFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType::OneMinusSrcAlpha);

    // Apply color mode blending if not replace
    if (!colorMode.blendFunction.is<gfx::ColorMode::Replace>()) {
        colorMode.blendFunction.match(
            [&](const auto& blendFunction) {
                colorBlend.operation = getWGPUBlendOperation(gfx::ColorBlendEquationType(blendFunction.equation));
                colorBlend.srcFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType(blendFunction.srcFactor));
                colorBlend.dstFactor = getWGPUBlendFactor(gfx::ColorBlendFactorType(blendFunction.dstFactor));
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
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_False;
    depthStencilState.depthCompare = WGPUCompareFunction_Always;
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

    return wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
}

// WebGPU blend operation conversion (similar to Metal's conversion functions)
WGPUBlendOperation ShaderProgram::getWGPUBlendOperation(gfx::ColorBlendEquationType equation) {
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

// WebGPU blend factor conversion (similar to Metal's conversion functions)
WGPUBlendFactor ShaderProgram::getWGPUBlendFactor(gfx::ColorBlendFactorType factor) {
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

// Vertex format conversion (unchanged)
WGPUVertexFormat ShaderProgram::getWGPUFormat(gfx::AttributeDataType type) {
    switch (type) {
        case gfx::AttributeDataType::Byte:
            return WGPUVertexFormat_Sint8;
        case gfx::AttributeDataType::Byte2:
            return WGPUVertexFormat_Sint8x2;
        case gfx::AttributeDataType::Byte4:
            return WGPUVertexFormat_Sint8x4;
        case gfx::AttributeDataType::UByte:
            return WGPUVertexFormat_Uint8;
        case gfx::AttributeDataType::UByte2:
            return WGPUVertexFormat_Uint8x2;
        case gfx::AttributeDataType::UByte4:
            return WGPUVertexFormat_Uint8x4;
        case gfx::AttributeDataType::Short:
            return WGPUVertexFormat_Sint16;
        case gfx::AttributeDataType::Short2:
            return WGPUVertexFormat_Sint16x2;
        case gfx::AttributeDataType::Short4:
            return WGPUVertexFormat_Sint16x4;
        case gfx::AttributeDataType::UShort:
            return WGPUVertexFormat_Uint16;
        case gfx::AttributeDataType::UShort2:
            return WGPUVertexFormat_Uint16x2;
        case gfx::AttributeDataType::UShort4:
            return WGPUVertexFormat_Uint16x4;
        case gfx::AttributeDataType::Int:
            return WGPUVertexFormat_Sint32;
        case gfx::AttributeDataType::Int2:
            return WGPUVertexFormat_Sint32x2;
        case gfx::AttributeDataType::Int3:
            return WGPUVertexFormat_Sint32x3;
        case gfx::AttributeDataType::Int4:
            return WGPUVertexFormat_Sint32x4;
        case gfx::AttributeDataType::UInt:
            return WGPUVertexFormat_Uint32;
        case gfx::AttributeDataType::UInt2:
            return WGPUVertexFormat_Uint32x2;
        case gfx::AttributeDataType::UInt3:
            return WGPUVertexFormat_Uint32x3;
        case gfx::AttributeDataType::UInt4:
            return WGPUVertexFormat_Uint32x4;
        case gfx::AttributeDataType::Float:
            return WGPUVertexFormat_Float32;
        case gfx::AttributeDataType::Float2:
            return WGPUVertexFormat_Float32x2;
        case gfx::AttributeDataType::Float3:
            return WGPUVertexFormat_Float32x3;
        case gfx::AttributeDataType::Float4:
            return WGPUVertexFormat_Float32x4;
        default:
            return WGPUVertexFormat_Float32x2;
    }
}

} // namespace webgpu
} // namespace mbgl
