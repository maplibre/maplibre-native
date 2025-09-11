#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

ShaderProgram::ShaderProgram(Context& context_,
                           const std::string& vertexSource,
                           const std::string& fragmentSource)
    : context(context_) {
    createPipeline(vertexSource, fragmentSource);
}

ShaderProgram::~ShaderProgram() {
    if (pipeline) {
        wgpuRenderPipelineRelease(pipeline);
        pipeline = nullptr;
    }
    if (bindGroupLayout) {
        wgpuBindGroupLayoutRelease(bindGroupLayout);
        // No bind group layout needed for testing
    }
    if (pipelineLayout) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        pipelineLayout = nullptr;
    }
    if (vertexShaderModule) {
        wgpuShaderModuleRelease(vertexShaderModule);
        vertexShaderModule = nullptr;
    }
    if (fragmentShaderModule) {
        wgpuShaderModuleRelease(fragmentShaderModule);
        fragmentShaderModule = nullptr;
    }
}

void ShaderProgram::createPipeline(const std::string& vertexSource, const std::string& fragmentSource) {
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    if (!device) {
        Log::Error(Event::General, "Failed to get WebGPU device for shader creation");
        return;
    }
    
    // Create vertex shader module
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = (WGPUSType)0x00040006;  // WGPUSType_ShaderModuleWGSLDescriptor
    wgslDesc.chain.next = nullptr;
    WGPUStringView vertexCode = {vertexSource.c_str(), vertexSource.length()};
    wgslDesc.code = vertexCode;
    
    WGPUShaderModuleDescriptor vertexShaderDesc = {};
    WGPUStringView vertexLabel = {"Vertex Shader Module", strlen("Vertex Shader Module")};
    vertexShaderDesc.label = vertexLabel;
    vertexShaderDesc.nextInChain = (WGPUChainedStruct*)&wgslDesc;
    
    vertexShaderModule = wgpuDeviceCreateShaderModule(device, &vertexShaderDesc);
    if (!vertexShaderModule) {
        Log::Error(Event::General, "Failed to create vertex shader module - WGSL syntax error?");
        Log::Error(Event::General, "Vertex shader first 200 chars: " + vertexSource.substr(0, 200));
        return;
    } else {
        Log::Info(Event::General, "Vertex shader module created successfully");
    }
    
    // Create fragment shader module
    WGPUStringView fragmentCode = {fragmentSource.c_str(), fragmentSource.length()};
    wgslDesc.code = fragmentCode;
    
    WGPUShaderModuleDescriptor fragmentShaderDesc = {};
    WGPUStringView fragmentLabel = {"Fragment Shader Module", strlen("Fragment Shader Module")};
    fragmentShaderDesc.label = fragmentLabel;
    fragmentShaderDesc.nextInChain = (WGPUChainedStruct*)&wgslDesc;
    
    fragmentShaderModule = wgpuDeviceCreateShaderModule(device, &fragmentShaderDesc);
    if (!fragmentShaderModule) {
        Log::Error(Event::General, "Failed to create fragment shader module - WGSL syntax error?");
        Log::Error(Event::General, "Fragment shader first 200 chars: " + fragmentSource.substr(0, 200));
        return;
    } else {
        Log::Info(Event::General, "Fragment shader module created successfully");
    }
    
    // Create pipeline layout with no bind groups (for testing)
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    WGPUStringView pipelineLayoutLabel = {"Pipeline Layout", strlen("Pipeline Layout")};
    pipelineLayoutDesc.label = pipelineLayoutLabel;
    pipelineLayoutDesc.bindGroupLayoutCount = 0;  // No bind groups
    pipelineLayoutDesc.bindGroupLayouts = nullptr;
    
    pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);
    
    // Set up vertex state - no vertex buffers for now (using hardcoded triangle)
    WGPUVertexState vertexState = {};
    vertexState.module = vertexShaderModule;
    WGPUStringView vertexState_entryPoint_str = {"main", strlen("main")};
    vertexState.entryPoint = vertexState_entryPoint_str;
    vertexState.bufferCount = 0;  // No vertex buffers
    vertexState.buffers = nullptr;
    
    // Set up fragment state with alpha blending
    WGPUBlendComponent alphaBlend = {};
    alphaBlend.operation = WGPUBlendOperation_Add;
    alphaBlend.srcFactor = WGPUBlendFactor_SrcAlpha;
    alphaBlend.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    
    WGPUBlendComponent colorBlend = {};
    colorBlend.operation = WGPUBlendOperation_Add;
    colorBlend.srcFactor = WGPUBlendFactor_SrcAlpha;
    colorBlend.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    
    WGPUBlendState blendState = {};
    blendState.color = colorBlend;
    blendState.alpha = alphaBlend;
    
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm; // Default format, should match surface
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    WGPUFragmentState fragmentState = {};
    fragmentState.module = fragmentShaderModule;
    WGPUStringView fragmentState_entryPoint_str = {"main", strlen("main")};
    fragmentState.entryPoint = fragmentState_entryPoint_str;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    
    // Set up primitive state
    WGPUPrimitiveState primitiveState = {};
    primitiveState.topology = WGPUPrimitiveTopology_TriangleList;
    primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitiveState.frontFace = WGPUFrontFace_CCW;
    primitiveState.cullMode = WGPUCullMode_None;
    
    // Set up depth stencil state - disable depth testing for now
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
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0.0f;
    depthStencilState.depthBiasClamp = 0.0f;
    
    // Create render pipeline
    WGPURenderPipelineDescriptor pipelineDesc = {};
    WGPUStringView pipelineLabel = {"Render Pipeline", strlen("Render Pipeline")};
    pipelineDesc.label = pipelineLabel;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitiveState;
    pipelineDesc.depthStencil = nullptr; // No depth-stencil for now
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = 0;
    
    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
    if (!pipeline) {
        Log::Error(Event::General, "Failed to create WebGPU render pipeline - shader compilation or pipeline configuration error");
        Log::Error(Event::General, "Vertex shader length: " + std::to_string(vertexSource.length()));
        Log::Error(Event::General, "Fragment shader length: " + std::to_string(fragmentSource.length()));
        Log::Error(Event::General, "Vertex shader module: " + std::to_string(vertexShaderModule != nullptr));
        Log::Error(Event::General, "Fragment shader module: " + std::to_string(fragmentShaderModule != nullptr));
    } else {
        uintptr_t pipelineAddr = reinterpret_cast<uintptr_t>(pipeline);
        Log::Info(Event::General, "Successfully created WebGPU render pipeline at address: 0x" + 
                  std::to_string(pipelineAddr));
        Log::Info(Event::General, "Pipeline created with vertex buffers: " + std::to_string(vertexState.bufferCount));
    }
}

} // namespace webgpu
} // namespace mbgl