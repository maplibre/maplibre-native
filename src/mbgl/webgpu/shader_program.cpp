#include <mbgl/webgpu/shader_program.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/util/logging.hpp>
#include <functional>

namespace mbgl {
namespace webgpu {

ShaderProgram::ShaderProgram(Context& context_,
                           const std::string& name,
                           const std::string& vertexSource,
                           const std::string& fragmentSource)
    : gfx::ShaderProgramBase(name),
      context(context_) {
    compileShader(vertexSource, fragmentSource);
    parseBindings(vertexSource + fragmentSource);
    createPipelineLayout();
}

ShaderProgram::~ShaderProgram() {
    // Clean up pipelines
    for (auto& [key, pipeline] : pipelineCache) {
        if (pipeline) {
            // wgpuRenderPipelineRelease(pipeline);
        }
    }
    
    // Clean up bind group layouts
    for (auto& layout : bindGroupLayouts) {
        if (layout) {
            // wgpuBindGroupLayoutRelease(layout);
        }
    }
    
    if (pipelineLayout) {
        // wgpuPipelineLayoutRelease(pipelineLayout);
    }
    
    if (fragmentModule) {
        // wgpuShaderModuleRelease(fragmentModule);
    }
    
    if (vertexModule) {
        // wgpuShaderModuleRelease(vertexModule);
    }
}

void ShaderProgram::compileShader(const std::string& vertexSource, const std::string& fragmentSource) {
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    // Create vertex shader module
    WGPUShaderModuleDescriptor vertexDesc = {};
    vertexDesc.label = (getName() + " Vertex").c_str();
    
    // WebGPU uses WGSL (WebGPU Shading Language)
    // For now, we'll assume the sources are already in WGSL format
    // In production, you'd want to convert from GLSL to WGSL
    WGPUShaderModuleWGSLDescriptor vertexWgsl = {};
    vertexWgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    vertexWgsl.code = vertexSource.c_str();
    vertexDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&vertexWgsl);
    
    // vertexModule = wgpuDeviceCreateShaderModule(device, &vertexDesc);
    
    if (!vertexModule) {
        Log::Error(Event::Render, "Failed to create vertex shader module for " + getName());
    }
    
    // Create fragment shader module
    WGPUShaderModuleDescriptor fragmentDesc = {};
    fragmentDesc.label = (getName() + " Fragment").c_str();
    
    WGPUShaderModuleWGSLDescriptor fragmentWgsl = {};
    fragmentWgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    fragmentWgsl.code = fragmentSource.c_str();
    fragmentDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&fragmentWgsl);
    
    // fragmentModule = wgpuDeviceCreateShaderModule(device, &fragmentDesc);
    
    if (!fragmentModule) {
        Log::Error(Event::Render, "Failed to create fragment shader module for " + getName());
    }
}

void ShaderProgram::parseBindings(const std::string& source) {
    // Parse WGSL source to extract binding information
    // This is a simplified parser - in production you'd want a proper WGSL parser
    
    // Look for @group(X) @binding(Y) declarations
    size_t pos = 0;
    while ((pos = source.find("@group(", pos)) != std::string::npos) {
        size_t groupStart = pos + 7;
        size_t groupEnd = source.find(")", groupStart);
        if (groupEnd == std::string::npos) break;
        
        uint32_t group = std::stoul(source.substr(groupStart, groupEnd - groupStart));
        
        size_t bindingPos = source.find("@binding(", groupEnd);
        if (bindingPos == std::string::npos || bindingPos - groupEnd > 50) {
            pos = groupEnd;
            continue;
        }
        
        size_t bindingStart = bindingPos + 9;
        size_t bindingEnd = source.find(")", bindingStart);
        if (bindingEnd == std::string::npos) break;
        
        uint32_t binding = std::stoul(source.substr(bindingStart, bindingEnd - bindingStart));
        
        // Find the variable declaration
        size_t varPos = source.find("var", bindingEnd);
        if (varPos != std::string::npos && varPos - bindingEnd < 100) {
            // Determine if it's a uniform buffer or texture
            if (source.find("uniform", varPos) != std::string::npos && 
                source.find("uniform", varPos) - varPos < 20) {
                // It's a uniform buffer
                BindingInfo info;
                info.group = group;
                info.binding = binding;
                info.type = WGPUBufferBindingType_Uniform;
                info.minBindingSize = 0; // Will be determined later
                
                // Extract variable name (simplified)
                size_t nameStart = source.find(":", varPos);
                if (nameStart != std::string::npos) {
                    nameStart = source.find_last_of(" ", nameStart) + 1;
                    size_t nameEnd = source.find_first_of(":;", nameStart);
                    std::string name = source.substr(nameStart, nameEnd - nameStart);
                    uniformBindings[name] = info;
                }
            } else if (source.find("texture", varPos) != std::string::npos &&
                      source.find("texture", varPos) - varPos < 20) {
                // It's a texture
                BindingInfo info;
                info.group = group;
                info.binding = binding;
                info.type = WGPUBufferBindingType_Undefined; // Not a buffer
                info.minBindingSize = 0;
                
                // Extract variable name (simplified)
                size_t nameStart = source.find(":", varPos);
                if (nameStart != std::string::npos) {
                    nameStart = source.find_last_of(" ", nameStart) + 1;
                    size_t nameEnd = source.find_first_of(":;", nameStart);
                    std::string name = source.substr(nameStart, nameEnd - nameStart);
                    textureBindings[name] = info;
                }
            }
        }
        
        pos = bindingEnd;
    }
}

void ShaderProgram::createPipelineLayout() {
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    // Create bind group layouts based on parsed bindings
    // Group bindings by group number
    std::map<uint32_t, std::vector<WGPUBindGroupLayoutEntry>> groupEntries;
    
    for (const auto& [name, info] : uniformBindings) {
        WGPUBindGroupLayoutEntry entry = {};
        entry.binding = info.binding;
        entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
        entry.buffer.type = info.type;
        entry.buffer.hasDynamicOffset = false;
        entry.buffer.minBindingSize = info.minBindingSize;
        groupEntries[info.group].push_back(entry);
    }
    
    for (const auto& [name, info] : textureBindings) {
        WGPUBindGroupLayoutEntry entry = {};
        entry.binding = info.binding;
        entry.visibility = WGPUShaderStage_Fragment;
        entry.texture.sampleType = WGPUTextureSampleType_Float;
        entry.texture.viewDimension = WGPUTextureViewDimension_2D;
        entry.texture.multisampled = false;
        groupEntries[info.group].push_back(entry);
        
        // Add sampler entry (typically binding + 1)
        WGPUBindGroupLayoutEntry samplerEntry = {};
        samplerEntry.binding = info.binding + 1;
        samplerEntry.visibility = WGPUShaderStage_Fragment;
        samplerEntry.sampler.type = WGPUSamplerBindingType_Filtering;
        groupEntries[info.group].push_back(samplerEntry);
    }
    
    // Create bind group layouts
    bindGroupLayouts.clear();
    for (const auto& [group, entries] : groupEntries) {
        WGPUBindGroupLayoutDescriptor desc = {};
        desc.label = (getName() + " Bind Group Layout " + std::to_string(group)).c_str();
        desc.entryCount = entries.size();
        desc.entries = entries.data();
        
        // WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(device, &desc);
        // bindGroupLayouts.push_back(layout);
    }
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.label = (getName() + " Pipeline Layout").c_str();
    layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts = bindGroupLayouts.data();
    
    // pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);
}

WGPURenderPipeline ShaderProgram::getOrCreatePipeline(const gfx::ColorMode& colorMode,
                                                      const gfx::CullFaceMode& cullFaceMode,
                                                      const gfx::DepthMode& depthMode,
                                                      const gfx::StencilMode& stencilMode,
                                                      const gfx::VertexAttributeArray& attributes) {
    // Create pipeline key
    PipelineKey key;
    key.colorMode = colorMode;
    key.cullFaceMode = cullFaceMode;
    key.depthMode = depthMode;
    key.stencilMode = stencilMode;
    
    // Calculate vertex layout hash
    std::hash<size_t> hasher;
    key.vertexLayoutHash = 0;
    for (const auto& attr : attributes) {
        key.vertexLayoutHash ^= hasher(static_cast<size_t>(attr->getIndex())) + 0x9e3779b9 + (key.vertexLayoutHash << 6) + (key.vertexLayoutHash >> 2);
    }
    
    // Check cache
    auto it = pipelineCache.find(key);
    if (it != pipelineCache.end()) {
        return it->second;
    }
    
    // Create new pipeline
    auto* impl = context.getImpl();
    if (!impl) {
        return nullptr;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return nullptr;
    }
    
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.label = getName().c_str();
    pipelineDesc.layout = pipelineLayout;
    
    // Vertex stage
    pipelineDesc.vertex.module = vertexModule;
    pipelineDesc.vertex.entryPoint = "main";
    
    // Set up vertex attributes and buffers
    std::vector<WGPUVertexAttribute> vertexAttributes;
    for (const auto& attr : attributes) {
        WGPUVertexAttribute wgpuAttr = {};
        wgpuAttr.format = WGPUVertexFormat_Float32x2; // TODO: Map from attr type
        wgpuAttr.offset = attr->getOffset();
        wgpuAttr.shaderLocation = attr->getIndex();
        vertexAttributes.push_back(wgpuAttr);
    }
    
    WGPUVertexBufferLayout vertexBuffer = {};
    vertexBuffer.arrayStride = 0; // TODO: Calculate stride
    vertexBuffer.stepMode = WGPUVertexStepMode_Vertex;
    vertexBuffer.attributeCount = vertexAttributes.size();
    vertexBuffer.attributes = vertexAttributes.data();
    
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBuffer;
    
    // Fragment stage
    WGPUFragmentState fragment = {};
    fragment.module = fragmentModule;
    fragment.entryPoint = "main";
    
    // Color target
    WGPUBlendState blend = {};
    if (colorMode.blending) {
        blend.color.operation = WGPUBlendOperation_Add;
        blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
        blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
        blend.alpha.operation = WGPUBlendOperation_Add;
        blend.alpha.srcFactor = WGPUBlendFactor_One;
        blend.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    }
    
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm; // TODO: Get from context
    colorTarget.blend = colorMode.blending ? &blend : nullptr;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    pipelineDesc.fragment = &fragment;
    
    // Primitive state
    WGPUPrimitiveState primitive = {};
    primitive.topology = WGPUPrimitiveTopology_TriangleList;
    primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitive.frontFace = WGPUFrontFace_CCW;
    
    switch (cullFaceMode.type) {
        case gfx::CullFaceType::None:
            primitive.cullMode = WGPUCullMode_None;
            break;
        case gfx::CullFaceType::Front:
            primitive.cullMode = WGPUCullMode_Front;
            break;
        case gfx::CullFaceType::Back:
            primitive.cullMode = WGPUCullMode_Back;
            break;
    }
    
    pipelineDesc.primitive = primitive;
    
    // Depth/stencil state
    WGPUDepthStencilState depthStencil = {};
    if (depthMode.enabled) {
        depthStencil.format = WGPUTextureFormat_Depth24PlusStencil8;
        depthStencil.depthWriteEnabled = depthMode.mask == gfx::DepthMaskType::ReadWrite;
        depthStencil.depthCompare = WGPUCompareFunction_Less;
    }
    
    if (stencilMode.enabled) {
        // TODO: Configure stencil state
    }
    
    if (depthMode.enabled || stencilMode.enabled) {
        pipelineDesc.depthStencil = &depthStencil;
    }
    
    // Multisample state
    WGPUMultisampleState multisample = {};
    multisample.count = 1;
    multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample = multisample;
    
    // Create pipeline
    // WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
    WGPURenderPipeline pipeline = nullptr; // Placeholder
    
    // Cache the pipeline
    pipelineCache[key] = pipeline;
    
    return pipeline;
}

bool ShaderProgram::PipelineKey::operator==(const PipelineKey& other) const {
    return colorMode == other.colorMode &&
           cullFaceMode == other.cullFaceMode &&
           depthMode == other.depthMode &&
           stencilMode == other.stencilMode &&
           vertexLayoutHash == other.vertexLayoutHash;
}

size_t ShaderProgram::PipelineKeyHash::operator()(const PipelineKey& key) const {
    size_t hash = 0;
    hash ^= std::hash<int>()(static_cast<int>(key.colorMode.blending)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<int>()(static_cast<int>(key.cullFaceMode.type)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<bool>()(key.depthMode.enabled) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<bool>()(key.stencilMode.enabled) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= key.vertexLayoutHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

} // namespace webgpu
} // namespace mbgl