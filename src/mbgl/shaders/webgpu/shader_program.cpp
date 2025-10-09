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
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <type_traits>
#include <mbgl/util/variant.hpp>

namespace mbgl {
namespace webgpu {

namespace {
std::string trimString(std::string_view value) {
    auto begin = value.begin();
    auto end = value.end();
    while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    while (begin != end) {
        auto prev = end - 1;
        if (!std::isspace(static_cast<unsigned char>(*prev))) {
            break;
        }
        end = prev;
    }
    return std::string(begin, end);
}

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

WGPUPrimitiveTopology toPrimitiveTopology(gfx::DrawModeType drawModeType) {
    switch (drawModeType) {
        case gfx::DrawModeType::Points:
            return WGPUPrimitiveTopology_PointList;
        case gfx::DrawModeType::Lines:
        case gfx::DrawModeType::LineLoop:
            return WGPUPrimitiveTopology_LineList;
        case gfx::DrawModeType::LineStrip:
            return WGPUPrimitiveTopology_LineStrip;
        case gfx::DrawModeType::TriangleStrip:
            return WGPUPrimitiveTopology_TriangleStrip;
        case gfx::DrawModeType::TriangleFan:
            // WebGPU does not support triangle fan directly; approximate with triangle list
            return WGPUPrimitiveTopology_TriangleList;
        case gfx::DrawModeType::Triangles:
        default:
            return WGPUPrimitiveTopology_TriangleList;
    }
}

WGPUCompareFunction toCompareFunction(const gfx::DepthFunctionType func) {
    switch (func) {
        case gfx::DepthFunctionType::Never:
            return WGPUCompareFunction_Never;
        case gfx::DepthFunctionType::Less:
            return WGPUCompareFunction_Less;
        case gfx::DepthFunctionType::Equal:
            return WGPUCompareFunction_Equal;
        case gfx::DepthFunctionType::LessEqual:
            return WGPUCompareFunction_LessEqual;
        case gfx::DepthFunctionType::Greater:
            return WGPUCompareFunction_Greater;
        case gfx::DepthFunctionType::NotEqual:
            return WGPUCompareFunction_NotEqual;
        case gfx::DepthFunctionType::GreaterEqual:
            return WGPUCompareFunction_GreaterEqual;
        case gfx::DepthFunctionType::Always:
        default:
            return WGPUCompareFunction_Always;
    }
}

WGPUCompareFunction toCompareFunction(const gfx::StencilFunctionType func) {
    switch (func) {
        case gfx::StencilFunctionType::Never:
            return WGPUCompareFunction_Never;
        case gfx::StencilFunctionType::Less:
            return WGPUCompareFunction_Less;
        case gfx::StencilFunctionType::Equal:
            return WGPUCompareFunction_Equal;
        case gfx::StencilFunctionType::LessEqual:
            return WGPUCompareFunction_LessEqual;
        case gfx::StencilFunctionType::Greater:
            return WGPUCompareFunction_Greater;
        case gfx::StencilFunctionType::NotEqual:
            return WGPUCompareFunction_NotEqual;
        case gfx::StencilFunctionType::GreaterEqual:
            return WGPUCompareFunction_GreaterEqual;
        case gfx::StencilFunctionType::Always:
        default:
            return WGPUCompareFunction_Always;
    }
}

WGPUStencilOperation toStencilOp(const gfx::StencilOpType op) {
    switch (op) {
        case gfx::StencilOpType::Zero:
            return WGPUStencilOperation_Zero;
        case gfx::StencilOpType::Keep:
            return WGPUStencilOperation_Keep;
        case gfx::StencilOpType::Replace:
            return WGPUStencilOperation_Replace;
        case gfx::StencilOpType::Increment:
            return WGPUStencilOperation_IncrementClamp;
        case gfx::StencilOpType::Decrement:
            return WGPUStencilOperation_DecrementClamp;
        case gfx::StencilOpType::Invert:
            return WGPUStencilOperation_Invert;
        case gfx::StencilOpType::IncrementWrap:
            return WGPUStencilOperation_IncrementWrap;
        case gfx::StencilOpType::DecrementWrap:
            return WGPUStencilOperation_DecrementWrap;
        default:
            return WGPUStencilOperation_Keep;
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
    createPipelineLayout({}, {});
}

// Minimal constructor for Context::createShader compatibility
ShaderProgram::ShaderProgram(Context& context, const std::string& vertexSource, const std::string& fragmentSource)
    : ShaderProgramBase(),
      shaderName("ContextShader"),
      backend(static_cast<RendererBackend&>(context.getBackend())) {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (!device) return;

    std::string trimmedVertex = trimString(vertexSource);
    std::string trimmedFragment = trimString(fragmentSource);
    hasVertexEntryPoint = !trimmedVertex.empty();
    hasFragmentEntryPoint = !trimmedFragment.empty();

    // Get the prelude from common.hpp
    using PreludeShader = shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::WebGPU>;

    std::string vertexWithPrelude;
    if (hasVertexEntryPoint) {
        vertexWithPrelude = std::string(PreludeShader::prelude) + "\n" + vertexSource;
        WGPUShaderSourceWGSL wgslDesc = {};
        wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
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
            if (shaderName.find("CircleShader") != std::string::npos) {
                std::ofstream dump("/tmp/circle_vertex.wgsl", std::ios::trunc);
                dump << vertexSource;
            }
        }
    }

    std::string fragmentWithPrelude;
    if (hasFragmentEntryPoint) {
        fragmentWithPrelude = std::string(PreludeShader::prelude) + "\n" + fragmentSource;

        WGPUShaderSourceWGSL wgslDesc = {};
        wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgslDesc.chain.next = nullptr;
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
            if (shaderName.find("CircleShader") != std::string::npos) {
                std::ofstream dump("/tmp/circle_fragment.wgsl", std::ios::trunc);
                dump << fragmentSource;
            }
        }
    }

    createPipelineLayout(vertexWithPrelude, fragmentWithPrelude);
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

    for (auto layout : bindGroupLayouts) {
        if (layout) {
            wgpuBindGroupLayoutRelease(layout);
        }
    }
    bindGroupLayouts.clear();
    bindGroupOrder.clear();
    bindingsPerGroup.clear();

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
                                                    const gfx::DepthMode& depthMode,
                                                    const gfx::StencilMode& stencilMode,
                                                    gfx::DrawModeType drawModeType,
                                                    const std::optional<std::size_t> reuseHash) {
    (void)renderable;
    // Check cache first
    if (reuseHash.has_value()) {
        auto it = renderPipelineCache.find(reuseHash.value());
        if (it != renderPipelineCache.end()) {
            return it->second;
        }
    }

    // Create new pipeline
    WGPURenderPipeline pipeline = createPipeline(
        vertexLayouts, vertexLayoutCount, colorMode, depthMode, stencilMode, drawModeType);

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
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
    instanceAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
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

const std::vector<ShaderProgram::BindingInfo>& ShaderProgram::getBindingInfosForGroup(uint32_t group) const {
    for (size_t i = 0; i < bindGroupOrder.size(); ++i) {
        if (bindGroupOrder[i] == group) {
            return bindingsPerGroup[i];
        }
    }
    static const std::vector<BindingInfo> empty;
    return empty;
}

WGPUBindGroupLayout ShaderProgram::getBindGroupLayout(uint32_t group) const {
    for (size_t i = 0; i < bindGroupOrder.size(); ++i) {
        if (bindGroupOrder[i] == group) {
            return bindGroupLayouts[i];
        }
    }
    return nullptr;
}

void ShaderProgram::createPipelineLayout(const std::string& vertexSource, const std::string& fragmentSource) {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) {
        return;
    }

    if (!vertexSource.empty() || !fragmentSource.empty()) {
        bindingInfos.clear();
        if (!vertexSource.empty()) {
            analyzeShaderBindings(vertexSource, WGPUShaderStage_Vertex);
        }
        if (!fragmentSource.empty()) {
            analyzeShaderBindings(fragmentSource, WGPUShaderStage_Fragment);
        }
    }

    rebuildBindGroupLayouts();

    if (pipelineLayout) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        pipelineLayout = nullptr;
    }

    std::vector<WGPUBindGroupLayout> layoutHandles;
    layoutHandles.reserve(bindGroupLayouts.size());
    for (const auto& layout : bindGroupLayouts) {
        layoutHandles.push_back(layout);
    }

    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    WGPUStringView pipelineLayoutLabel = {"Pipeline Layout", strlen("Pipeline Layout")};
    pipelineLayoutDesc.label = pipelineLayoutLabel;
    pipelineLayoutDesc.bindGroupLayoutCount = layoutHandles.size();
    pipelineLayoutDesc.bindGroupLayouts = layoutHandles.empty() ? nullptr : layoutHandles.data();

    pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);
}

void ShaderProgram::analyzeShaderBindings(const std::string& source, WGPUShaderStage stage) {
    size_t searchPos = 0;
    while (true) {
        size_t groupToken = source.find("@group", searchPos);
        if (groupToken == std::string::npos) {
            break;
        }
        size_t groupParen = source.find('(', groupToken);
        if (groupParen == std::string::npos) {
            break;
        }
        size_t groupStart = groupParen + 1;
        while (groupStart < source.size() && std::isspace(static_cast<unsigned char>(source[groupStart]))) {
            ++groupStart;
        }
        size_t groupEnd = groupStart;
        while (groupEnd < source.size() && std::isdigit(static_cast<unsigned char>(source[groupEnd]))) {
            ++groupEnd;
        }
        if (groupEnd == groupStart) {
            searchPos = groupEnd;
            continue;
        }
        uint32_t group = static_cast<uint32_t>(
            std::strtoul(source.substr(groupStart, groupEnd - groupStart).c_str(), nullptr, 10));

        size_t bindingToken = source.find("@binding", groupEnd);
        if (bindingToken == std::string::npos) {
            break;
        }
        size_t bindingParen = source.find('(', bindingToken);
        if (bindingParen == std::string::npos) {
            break;
        }
        size_t bindingStart = bindingParen + 1;
        while (bindingStart < source.size() && std::isspace(static_cast<unsigned char>(source[bindingStart]))) {
            ++bindingStart;
        }
        size_t bindingEnd = bindingStart;
        while (bindingEnd < source.size() && std::isdigit(static_cast<unsigned char>(source[bindingEnd]))) {
            ++bindingEnd;
        }
        if (bindingEnd == bindingStart) {
            searchPos = bindingEnd;
            continue;
        }
        uint32_t binding = static_cast<uint32_t>(
            std::strtoul(source.substr(bindingStart, bindingEnd - bindingStart).c_str(), nullptr, 10));

        size_t varPos = source.find("var", bindingEnd);
        if (varPos == std::string::npos) {
            break;
        }
        if (varPos > bindingEnd && std::isalpha(static_cast<unsigned char>(source[varPos - 1]))) {
            searchPos = varPos + 3;
            continue;
        }

        size_t afterVar = varPos + 3;
        while (afterVar < source.size() && std::isspace(static_cast<unsigned char>(source[afterVar]))) {
            ++afterVar;
        }

        BindingType type = BindingType::UniformBuffer;
        if (afterVar < source.size() && source[afterVar] == '<') {
            size_t qualifierEnd = source.find('>', afterVar);
            if (qualifierEnd == std::string::npos) {
                break;
            }
            const auto qualifier = trimString(
                std::string_view(source.data() + afterVar + 1, qualifierEnd - afterVar - 1));
            if (qualifier.find("storage") != std::string::npos) {
                if (qualifier.find("read_write") != std::string::npos) {
                    type = BindingType::StorageBuffer;
                } else {
                    type = BindingType::ReadOnlyStorageBuffer;
                }
            } else {
                type = BindingType::UniformBuffer;
            }
            afterVar = qualifierEnd + 1;
        }

        size_t colonPos = source.find(':', afterVar);
        if (colonPos == std::string::npos) {
            break;
        }
        size_t typeStart = colonPos + 1;
        size_t typeEnd = typeStart;
        while (typeEnd < source.size()) {
            char c = source[typeEnd];
            if (c == ';' || c == '\n' || c == '\r') {
                break;
            }
            ++typeEnd;
        }
        const auto typeName = trimString(std::string_view(source.data() + typeStart, typeEnd - typeStart));

        if (type == BindingType::UniformBuffer) {
            if (typeName.find("sampler") != std::string::npos) {
                type = BindingType::Sampler;
            } else if (typeName.find("texture") != std::string::npos) {
                type = BindingType::Texture;
            }
        }

        auto existing = std::find_if(bindingInfos.begin(), bindingInfos.end(), [&](const BindingInfo& info) {
            return info.group == group && info.binding == binding;
        });

        if (existing == bindingInfos.end()) {
            bindingInfos.push_back({group, binding, type, stage});
        } else {
            existing->visibility |= stage;
            if (existing->type == BindingType::UniformBuffer && type != BindingType::UniformBuffer) {
                existing->type = type;
            }
        }

        searchPos = typeEnd;
    }
}

void ShaderProgram::rebuildBindGroupLayouts() {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (!device) {
        return;
    }

    for (auto layout : bindGroupLayouts) {
        if (layout) {
            wgpuBindGroupLayoutRelease(layout);
        }
    }
    bindGroupLayouts.clear();
    bindGroupOrder.clear();
    bindingsPerGroup.clear();

    if (bindingInfos.empty()) {
        bindingInfos.push_back({0u, 0u, BindingType::UniformBuffer, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment});
    }

    std::map<uint32_t, std::vector<BindingInfo>> grouped;
    for (const auto& info : bindingInfos) {
        grouped[info.group].push_back(info);
    }

    for (auto& entry : grouped) {
        auto& infos = entry.second;
        std::sort(infos.begin(), infos.end(), [](const BindingInfo& lhs, const BindingInfo& rhs) {
            return lhs.binding < rhs.binding;
        });

        std::vector<WGPUBindGroupLayoutEntry> layoutEntries;
        layoutEntries.reserve(infos.size());

        for (const auto& info : infos) {
            WGPUBindGroupLayoutEntry layoutEntry = {};
            layoutEntry.binding = info.binding;
            layoutEntry.visibility = info.visibility;

            switch (info.type) {
                case BindingType::UniformBuffer:
                    layoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
                    layoutEntry.buffer.hasDynamicOffset = 0;
                    layoutEntry.buffer.minBindingSize = 0;
                    break;
                case BindingType::ReadOnlyStorageBuffer:
                    layoutEntry.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                    layoutEntry.buffer.hasDynamicOffset = 0;
                    layoutEntry.buffer.minBindingSize = 0;
                    break;
                case BindingType::StorageBuffer:
                    layoutEntry.buffer.type = WGPUBufferBindingType_Storage;
                    layoutEntry.buffer.hasDynamicOffset = 0;
                    layoutEntry.buffer.minBindingSize = 0;
                    break;
                case BindingType::Sampler:
                    layoutEntry.sampler.type = WGPUSamplerBindingType_Filtering;
                    break;
                case BindingType::Texture:
                    layoutEntry.texture.sampleType = WGPUTextureSampleType_Float;
                    layoutEntry.texture.viewDimension = WGPUTextureViewDimension_2D;
                    layoutEntry.texture.multisampled = false;
                    break;
            }

            layoutEntries.push_back(layoutEntry);
        }

        WGPUBindGroupLayoutDescriptor desc = {};
        const std::string labelStr = "Shader Bind Group " + std::to_string(entry.first);
        WGPUStringView labelView = {labelStr.c_str(), labelStr.length()};
        desc.label = labelView;
        desc.entryCount = layoutEntries.size();
        desc.entries = layoutEntries.data();

        WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(device, &desc);
        if (layout) {
            bindGroupOrder.push_back(entry.first);
            bindingsPerGroup.push_back(infos);
            bindGroupLayouts.push_back(layout);
        }
    }
}

WGPURenderPipeline ShaderProgram::createPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                                 uint32_t vertexLayoutCount,
                                                 const gfx::ColorMode& colorMode,
                                                 const gfx::DepthMode& depthMode,
                                                 const gfx::StencilMode& stencilMode,
                                                 gfx::DrawModeType drawModeType) {
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!hasVertexEntryPoint || !hasFragmentEntryPoint) {
        if (!loggedMissingEntryPoint) {
            Log::Warning(
                Event::Render,
                "WebGPU: shader '" + shaderName + "' is missing WGSL entry points; skipping pipeline creation");
            loggedMissingEntryPoint = true;
        }
        return nullptr;
    }

    if (!device || !vertexShaderModule || !fragmentShaderModule || !pipelineLayout) {
        Log::Error(Event::Render,
                   "createPipeline missing requirement: device=" + std::to_string(device != nullptr) +
                       ", vertexModule=" + std::to_string(vertexShaderModule != nullptr) +
                       ", fragmentModule=" + std::to_string(fragmentShaderModule != nullptr) +
                       ", pipelineLayout=" + std::to_string(pipelineLayout != nullptr));
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
        colorMode.blendFunction.match([&](const auto& blendFunction) {
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
    auto colorFormat = backend.getColorFormat();
    if (colorFormat == wgpu::TextureFormat::Undefined) {
        colorFormat = wgpu::TextureFormat::BGRA8Unorm;
    }
    colorTarget.format = static_cast<WGPUTextureFormat>(colorFormat);
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
    primitiveState.topology = toPrimitiveTopology(drawModeType);
    primitiveState.stripIndexFormat = (drawModeType == gfx::DrawModeType::LineStrip ||
                                       drawModeType == gfx::DrawModeType::TriangleStrip)
                                          ? WGPUIndexFormat_Uint16
                                          : WGPUIndexFormat_Undefined;
    primitiveState.frontFace = WGPUFrontFace_CCW;
    primitiveState.cullMode = WGPUCullMode_None;

    // Set up depth stencil state
    WGPUDepthStencilState depthStencilState = {};
    const auto depthFormat = backend.getDepthStencilFormat();
    if (depthFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = static_cast<WGPUTextureFormat>(depthFormat);
        depthStencilState.depthWriteEnabled = depthMode.mask == gfx::DepthMaskType::ReadWrite ? WGPUOptionalBool_True
                                                                                              : WGPUOptionalBool_False;
        depthStencilState.depthCompare = toCompareFunction(depthMode.func);

        depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
        depthStencilState.stencilFront.failOp = WGPUStencilOperation_Keep;
        depthStencilState.stencilFront.depthFailOp = WGPUStencilOperation_Keep;
        depthStencilState.stencilFront.passOp = WGPUStencilOperation_Keep;
        depthStencilState.stencilBack = depthStencilState.stencilFront;
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;

        bool stencilEnabled = true;
        if (stencilMode.test.is<gfx::StencilMode::Always>() && stencilMode.mask == 0 &&
            stencilMode.fail == gfx::StencilOpType::Keep && stencilMode.depthFail == gfx::StencilOpType::Keep &&
            stencilMode.pass == gfx::StencilOpType::Keep) {
            stencilEnabled = false;
        }

        if (stencilEnabled) {
            gfx::StencilFunctionType function = gfx::StencilFunctionType::Always;
            uint32_t readMask = 0xFF;
            mapbox::util::apply_visitor(
                [&](const auto& test) {
                    using TestType = std::decay_t<decltype(test)>;
                    function = TestType::func;
                    if constexpr (requires { test.mask; }) {
                        readMask = test.mask;
                    } else {
                        readMask = 0xFF;
                    }
                },
                stencilMode.test);

            depthStencilState.stencilFront.compare = toCompareFunction(function);
            depthStencilState.stencilBack = depthStencilState.stencilFront;
            depthStencilState.stencilFront.failOp = toStencilOp(stencilMode.fail);
            depthStencilState.stencilFront.depthFailOp = toStencilOp(stencilMode.depthFail);
            depthStencilState.stencilFront.passOp = toStencilOp(stencilMode.pass);
            depthStencilState.stencilBack = depthStencilState.stencilFront;
            depthStencilState.stencilReadMask = readMask;
            depthStencilState.stencilWriteMask = stencilMode.mask;
        }

        if (shaderName == "ClippingMaskProgram") {
            depthStencilState.stencilFront.passOp = WGPUStencilOperation_Replace;
            depthStencilState.stencilBack = depthStencilState.stencilFront;
            if (depthStencilState.stencilWriteMask == 0) {
                depthStencilState.stencilWriteMask = 0xFF;
            }
            if (depthStencilState.stencilReadMask == 0) {
                depthStencilState.stencilReadMask = 0xFF;
            }
            depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
            depthStencilState.stencilBack = depthStencilState.stencilFront;
        }
    }

    // Create render pipeline
    WGPURenderPipelineDescriptor pipelineDesc = {};
    WGPUStringView pipelineLabel = {shaderName.c_str(), shaderName.length()};
    pipelineDesc.label = pipelineLabel;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitiveState;
    pipelineDesc.depthStencil = depthFormat != wgpu::TextureFormat::Undefined ? &depthStencilState : nullptr;
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
