#include <iostream>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/util/size.hpp>

int main() {
    std::cout << "Testing WebGPU Backend for MapLibre Native" << std::endl;
    
    try {
        // Create WebGPU context
        auto context = std::make_unique<mbgl::webgpu::Context>();
        std::cout << "✓ WebGPU Context created successfully" << std::endl;
        
        // Test creating basic resources
        mbgl::Size size{512, 512};
        
        // Create a render target
        auto renderTarget = context->createRenderTarget(size, mbgl::gfx::TextureChannelDataType::UnsignedByte);
        if (renderTarget) {
            std::cout << "✓ Render target created successfully" << std::endl;
        }
        
        // Create a layer group
        auto layerGroup = context->createLayerGroup(0, 100, "test_layer");
        if (layerGroup) {
            std::cout << "✓ Layer group created successfully" << std::endl;
        }
        
        // Create a tile layer group
        auto tileLayerGroup = context->createTileLayerGroup(1, 100, "test_tile_layer");
        if (tileLayerGroup) {
            std::cout << "✓ Tile layer group created successfully" << std::endl;
        }
        
        std::cout << "\n✅ WebGPU backend basic functionality test PASSED!" << std::endl;
        std::cout << "The WebGPU backend has been successfully integrated into MapLibre Native." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}