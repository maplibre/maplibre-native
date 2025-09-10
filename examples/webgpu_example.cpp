// WebGPU Example for MapLibre Native
// This example demonstrates how to use the WebGPU backend

#include <iostream>
#include <memory>

// Minimal includes to avoid complex dependencies
#include <mbgl/gfx/types.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

// For simplicity, we'll create a minimal example without GLFW
// In a real application, you would use GLFW or another windowing library

int main() {
    std::cout << "MapLibre Native WebGPU Example\n";
    std::cout << "==============================\n\n";
    
    try {
        // Create the WebGPU backend implementation
        auto backendImpl = mbgl::webgpu::BackendImpl::create();
        if (!backendImpl) {
            std::cerr << "Failed to create WebGPU backend implementation\n";
            return 1;
        }
        
        std::cout << "✓ Created WebGPU backend implementation\n";
        
        // Initialize the backend
        if (!backendImpl->initialize()) {
            std::cerr << "Failed to initialize WebGPU backend\n";
            return 1;
        }
        
        std::cout << "✓ Initialized WebGPU backend\n";
        
        // Create the renderer backend
        auto rendererBackend = std::make_unique<mbgl::webgpu::RendererBackend>(mbgl::gfx::ContextMode::Unique);
        
        std::cout << "✓ Created WebGPU renderer backend (800x600)\n";
        
        // Get the context
        auto& context = static_cast<mbgl::webgpu::Context&>(rendererBackend->getContext());
        
        std::cout << "✓ Got WebGPU context\n";
        
        // Create a command encoder
        auto commandEncoder = context.createCommandEncoder();
        if (!commandEncoder) {
            std::cerr << "Failed to create command encoder\n";
            return 1;
        }
        
        std::cout << "✓ Created command encoder\n";
        
        // In a real application, you would:
        // 1. Set up a window with GLFW or similar
        // 2. Create a surface for WebGPU
        // 3. Set up a render loop
        // 4. Create shaders and pipelines
        // 5. Render geometry
        
        std::cout << "\nWebGPU backend is working!\n";
        std::cout << "This is a minimal example. In a real application, you would:\n";
        std::cout << "- Set up a window with GLFW\n";
        std::cout << "- Create a WebGPU surface\n";
        std::cout << "- Set up a render loop\n";
        std::cout << "- Load and render map data\n";
        
        // Clean up
        backendImpl->shutdown();
        std::cout << "\n✓ Shutdown complete\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}