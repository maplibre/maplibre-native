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
    // Release resources in reverse order of creation
    // Pipeline depends on pipeline layout and shader modules
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
        // Log::Error(Event::Shader, "WebGPU ShaderProgram: Device is null");
        // Logging disabled to prevent heap corruption in multi-threaded context
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
        // Log::Error(Event::Shader, "WebGPU ShaderProgram: Failed to create vertex shader module");
        // Logging disabled to prevent heap corruption in multi-threaded context
        return;
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
        // Log::Error(Event::Shader, "WebGPU ShaderProgram: Failed to create fragment shader module");
        // Logging disabled to prevent heap corruption in multi-threaded context
        // Clean up vertex module before returning
        if (vertexShaderModule) {
            wgpuShaderModuleRelease(vertexShaderModule);
            vertexShaderModule = nullptr;
        }
        return;
    }

    // Create bind group layout for uniforms (MVP matrix)
    WGPUBindGroupLayoutEntry bindingEntry = {};
    bindingEntry.binding = 0;
    bindingEntry.visibility = WGPUShaderStage_Vertex;
    bindingEntry.buffer.type = WGPUBufferBindingType_Uniform;
    bindingEntry.buffer.hasDynamicOffset = 0;
    bindingEntry.buffer.minBindingSize = 64; // 4x4 matrix
    
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    WGPUStringView bindGroupLabel = {"Uniform Bind Group Layout", strlen("Uniform Bind Group Layout")};
    bindGroupLayoutDesc.label = bindGroupLabel;
    bindGroupLayoutDesc.entryCount = 1;
    bindGroupLayoutDesc.entries = &bindingEntry;
    
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);
    if (!bindGroupLayout) {
        // Log::Warning(Event::Shader, "WebGPU ShaderProgram: Failed to create bind group layout, using auto layout");
        // Logging disabled to prevent heap corruption in multi-threaded context
    }
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    WGPUStringView pipelineLayoutLabel = {"Pipeline Layout", strlen("Pipeline Layout")};
    pipelineLayoutDesc.label = pipelineLayoutLabel;
    
    if (bindGroupLayout) {
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    } else {
        // Use auto layout if bind group layout creation failed
        pipelineLayoutDesc.bindGroupLayoutCount = 0;
        pipelineLayoutDesc.bindGroupLayouts = nullptr;
    }

    pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);
    if (!pipelineLayout) {
        // Log::Error(Event::Shader, "WebGPU ShaderProgram: Failed to create pipeline layout");
        // Logging disabled to prevent heap corruption in multi-threaded context
        // Clean up resources
        if (bindGroupLayout) {
            wgpuBindGroupLayoutRelease(bindGroupLayout);
            bindGroupLayout = nullptr;
        }
        if (vertexShaderModule) {
            wgpuShaderModuleRelease(vertexShaderModule);
            vertexShaderModule = nullptr;
        }
        if (fragmentShaderModule) {
            wgpuShaderModuleRelease(fragmentShaderModule);
            fragmentShaderModule = nullptr;
        }
        return;
    }

    // Set up vertex state with vertex buffer for position data
    WGPUVertexAttribute vertexAttribute = {};
    vertexAttribute.format = WGPUVertexFormat_Sint16x2;  // 2 int16 values for x,y
    vertexAttribute.offset = 0;
    vertexAttribute.shaderLocation = 0;  // @location(0) in shader
    
    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.arrayStride = 4;  // 2 * sizeof(int16)
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = 1;
    vertexBufferLayout.attributes = &vertexAttribute;
    
    WGPUVertexState vertexState = {};
    vertexState.module = vertexShaderModule;
    WGPUStringView vertexState_entryPoint_str = {"main", strlen("main")};
    vertexState.entryPoint = vertexState_entryPoint_str;
    vertexState.bufferCount = 1;  // One vertex buffer
    vertexState.buffers = &vertexBufferLayout;

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
        mbgl::Log::Error(mbgl::Event::Shader, "WebGPU ShaderProgram: Failed to create render pipeline");
        // Clean up all resources on failure
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
    } else {
        mbgl::Log::Info(mbgl::Event::Shader, "WebGPU ShaderProgram: Successfully created render pipeline");
    }
}

} // namespace webgpu
} // namespace mbgl
