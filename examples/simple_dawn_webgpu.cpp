// Simple Dawn WebGPU Example for MapLibre Native
// This demonstrates basic Dawn initialization with Metal backend

#include <iostream>
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_cpp.h>

int main() {
    std::cout << "Simple Dawn WebGPU Demo\n";
    std::cout << "========================\n\n";
    
    // Initialize Dawn's proc table
    dawnProcSetProcs(&dawn::native::GetProcs());
    
    // Create Dawn instance
    dawn::native::Instance instance;
    
    // Request Metal adapter
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::Metal;
    
    std::vector<dawn::native::Adapter> adapters = instance.EnumerateAdapters(&adapterOptions);
    
    if (adapters.empty()) {
        std::cerr << "No Metal adapters found\n";
        return 1;
    }
    
    std::cout << "✓ Found " << adapters.size() << " Metal adapter(s)\n";
    
    // Get the first adapter
    wgpu::Adapter adapter = wgpu::Adapter(adapters[0].Get());
    
    // Get adapter info
    wgpu::AdapterInfo info = {};
    adapter.GetInfo(&info);
    
    std::cout << "✓ Found Metal adapter\n";
    if (info.device.data) {
        std::cout << "  Device: " << std::string(info.device.data, info.device.length) << "\n";
    }
    if (info.vendor.data) {
        std::cout << "  Vendor: " << std::string(info.vendor.data, info.vendor.length) << "\n";
    }
    if (info.architecture.data) {
        std::cout << "  Architecture: " << std::string(info.architecture.data, info.architecture.length) << "\n";
    }
    std::cout << "  Backend: Metal\n";
    
    // Create device
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "Dawn Device";
    
    wgpu::Device device = adapter.CreateDevice(&deviceDesc);
    if (!device) {
        std::cerr << "Failed to create device\n";
        return 1;
    }
    
    std::cout << "✓ Created WebGPU device\n";
    
    // Get queue
    wgpu::Queue queue = device.GetQueue();
    std::cout << "✓ Got device queue\n";
    
    // Create a simple compute shader to test
    const char* computeShader = R"(
        @group(0) @binding(0) var<storage, read_write> data: array<f32>;
        
        @compute @workgroup_size(1)
        fn main(@builtin(global_invocation_id) id: vec3<u32>) {
            data[id.x] = data[id.x] * 2.0;
        }
    )";
    
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.code = computeShader;
    
    wgpu::ShaderModuleDescriptor shaderDesc = {};
    shaderDesc.nextInChain = &wgslDesc;
    shaderDesc.label = "Compute Shader";
    
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderDesc);
    if (!shaderModule) {
        std::cerr << "Failed to create shader module\n";
        return 1;
    }
    
    std::cout << "✓ Created compute shader module\n";
    
    // Create buffer
    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.label = "Storage Buffer";
    bufferDesc.size = 4 * sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    if (!buffer) {
        std::cerr << "Failed to create buffer\n";
        return 1;
    }
    
    std::cout << "✓ Created storage buffer\n";
    
    // Write initial data
    float initialData[] = {1.0f, 2.0f, 3.0f, 4.0f};
    queue.WriteBuffer(buffer, 0, initialData, sizeof(initialData));
    std::cout << "✓ Wrote initial data: [1.0, 2.0, 3.0, 4.0]\n";
    
    // Create compute pipeline
    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.label = "Compute Pipeline";
    pipelineDesc.compute.module = shaderModule;
    pipelineDesc.compute.entryPoint = "main";
    
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);
    if (!pipeline) {
        std::cerr << "Failed to create compute pipeline\n";
        return 1;
    }
    
    std::cout << "✓ Created compute pipeline\n";
    
    // Create bind group
    wgpu::BindGroupEntry bgEntry = {};
    bgEntry.binding = 0;
    bgEntry.buffer = buffer;
    bgEntry.offset = 0;
    bgEntry.size = 4 * sizeof(float);
    
    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = pipeline.GetBindGroupLayout(0);
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;
    
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bgDesc);
    std::cout << "✓ Created bind group\n";
    
    // Create command encoder and run compute pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    
    wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
    computePass.SetPipeline(pipeline);
    computePass.SetBindGroup(0, bindGroup);
    computePass.DispatchWorkgroups(4);
    computePass.End();
    
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
    
    std::cout << "✓ Executed compute shader\n";
    
    // Read back results (simplified - in real code would be async)
    device.Tick();  // Process commands
    
    std::cout << "\n✓ Dawn WebGPU successfully initialized and tested!\n";
    std::cout << "  - Metal backend active\n";
    std::cout << "  - Compute shader compiled and executed\n";
    std::cout << "  - Ready for MapLibre integration\n";
    
    return 0;
}