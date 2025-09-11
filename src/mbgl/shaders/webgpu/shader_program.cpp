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
        bindGroupLayout = nullptr;
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
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = vertexSource.c_str();
    
    WGPUShaderModuleDescriptor vertexShaderDesc = {};
    vertexShaderDesc.label = "Vertex Shader Module";
    vertexShaderDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    
    vertexShaderModule = wgpuDeviceCreateShaderModule(device, &vertexShaderDesc);
    if (!vertexShaderModule) {
        Log::Error(Event::General, "Failed to create vertex shader module");
        return;
    }
    
    // Create fragment shader module
    wgslDesc.code = fragmentSource.c_str();
    
    WGPUShaderModuleDescriptor fragmentShaderDesc = {};
    fragmentShaderDesc.label = "Fragment Shader Module";
    fragmentShaderDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    
    fragmentShaderModule = wgpuDeviceCreateShaderModule(device, &fragmentShaderDesc);
    if (!fragmentShaderModule) {
        Log::Error(Event::General, "Failed to create fragment shader module");
        return;
    }
    
    // Create bind group layout for uniforms and textures
    // For now, create an empty bind group layout
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    bindGroupLayoutDesc.label = "Bind Group Layout";
    bindGroupLayoutDesc.entryCount = 0;
    bindGroupLayoutDesc.entries = nullptr;
    
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.label = "Pipeline Layout";
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    
    pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);
    
    // Set up vertex state
    WGPUVertexState vertexState = {};
    vertexState.module = vertexShaderModule;
    vertexState.entryPoint = "main";
    vertexState.bufferCount = 0;
    vertexState.buffers = nullptr;
    
    // Set up fragment state
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm; // Default format, should match surface
    colorTarget.blend = nullptr; // No blending for now
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    WGPUFragmentState fragmentState = {};
    fragmentState.module = fragmentShaderModule;
    fragmentState.entryPoint = "main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    
    // Set up primitive state
    WGPUPrimitiveState primitiveState = {};
    primitiveState.topology = WGPUPrimitiveTopology_TriangleList;
    primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitiveState.frontFace = WGPUFrontFace_CCW;
    primitiveState.cullMode = WGPUCullMode_None;
    
    // Set up depth stencil state (optional)
    WGPUDepthStencilState depthStencilState = {};
    depthStencilState.format = WGPUTextureFormat_Depth24PlusStencil8;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
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
    pipelineDesc.label = "Render Pipeline";
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitiveState;
    pipelineDesc.depthStencil = &depthStencilState;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    
    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
    if (!pipeline) {
        Log::Error(Event::General, "Failed to create WebGPU render pipeline");
    } else {
        Log::Debug(Event::General, "Successfully created WebGPU render pipeline");
    }
}

} // namespace webgpu
} // namespace mbgl