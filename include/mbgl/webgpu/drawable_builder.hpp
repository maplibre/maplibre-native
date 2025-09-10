#pragma once

#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <memory>
#include <string>

namespace mbgl {
namespace webgpu {

class Context;

class DrawableBuilder : public gfx::DrawableBuilder {
public:
    explicit DrawableBuilder(Context& context, std::string name);
    ~DrawableBuilder() override;

    // Build the final drawable
    std::unique_ptr<gfx::Drawable> build() override;
    
    // Configuration methods
    DrawableBuilder& setShader(gfx::ShaderProgramBasePtr shader) override;
    DrawableBuilder& setVertexAttributes(gfx::VertexAttributeArrayPtr attributes) override;
    DrawableBuilder& setVertexData(std::vector<uint8_t>&& data, std::size_t vertexCount) override;
    DrawableBuilder& setIndexData(gfx::IndexVectorBasePtr indices, std::vector<gfx::Drawable::UniqueDrawSegment> segments) override;
    DrawableBuilder& setTextures(const gfx::Drawable::Textures& textures) override;
    DrawableBuilder& setUniformBuffers(gfx::UniformBufferArrayPtr uniformBuffers) override;
    
    // Render state configuration
    DrawableBuilder& setEnableColor(bool value) override;
    DrawableBuilder& setColorMode(const gfx::ColorMode& value) override;
    DrawableBuilder& setEnableDepth(bool value) override;
    DrawableBuilder& setDepthType(gfx::DepthMaskType value) override;
    DrawableBuilder& setCullFaceMode(const gfx::CullFaceMode& value) override;
    DrawableBuilder& setLineWidth(float value) override;
    
    // Draw configuration
    DrawableBuilder& setDrawMode(gfx::DrawMode mode) override;
    DrawableBuilder& setVertexOffset(std::size_t offset) override;
    DrawableBuilder& setIndexOffset(std::size_t offset) override;
    DrawableBuilder& setInstanceCount(std::size_t count) override;
    
    // Layer configuration
    DrawableBuilder& setRenderPass(RenderPass pass) override;
    DrawableBuilder& setSubLayerIndex(int32_t index) override;
    DrawableBuilder& setLayerTweaker(LayerTweakerPtr tweaker) override;
    
    // Tile configuration
    DrawableBuilder& setTileID(OverscaledTileID tileID) override;
    DrawableBuilder& setSortKey(float key) override;
    
private:
    Context& context;
    std::unique_ptr<Drawable> drawable;
    
    // Temporary storage for configuration
    struct Config {
        gfx::ShaderProgramBasePtr shader;
        gfx::VertexAttributeArrayPtr vertexAttributes;
        std::vector<uint8_t> vertexData;
        std::size_t vertexCount = 0;
        gfx::IndexVectorBasePtr indices;
        std::vector<gfx::Drawable::UniqueDrawSegment> segments;
        gfx::Drawable::Textures textures;
        gfx::UniformBufferArrayPtr uniformBuffers;
        
        bool enableColor = true;
        gfx::ColorMode colorMode;
        bool enableDepth = true;
        gfx::DepthMaskType depthType = gfx::DepthMaskType::ReadWrite;
        gfx::CullFaceMode cullFaceMode;
        float lineWidth = 1.0f;
        
        gfx::DrawMode drawMode;
        std::size_t vertexOffset = 0;
        std::size_t indexOffset = 0;
        std::size_t instanceCount = 1;
        
        RenderPass renderPass = RenderPass::Opaque;
        int32_t subLayerIndex = 0;
        LayerTweakerPtr layerTweaker;
        
        std::optional<OverscaledTileID> tileID;
        float sortKey = 0.0f;
    } config;
};

} // namespace webgpu
} // namespace mbgl