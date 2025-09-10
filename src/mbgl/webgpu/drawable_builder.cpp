#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

DrawableBuilder::DrawableBuilder(Context& context_, std::string name)
    : context(context_),
      drawable(std::make_unique<Drawable>(std::move(name))) {
}

DrawableBuilder::~DrawableBuilder() = default;

std::unique_ptr<gfx::Drawable> DrawableBuilder::build() {
    if (!drawable) {
        Log::Error(Event::Render, "Drawable already built");
        return nullptr;
    }
    
    // Apply all configuration to the drawable
    if (config.shader) {
        drawable->setShader(std::move(config.shader));
    }
    
    if (config.vertexAttributes) {
        drawable->setVertexAttributes(std::move(config.vertexAttributes));
    }
    
    if (!config.vertexData.empty()) {
        drawable->setVertices(std::move(config.vertexData), config.vertexCount, gfx::AttributeDataType::Float);
    }
    
    if (config.indices) {
        drawable->setIndexData(std::move(config.indices), std::move(config.segments));
    }
    
    drawable->setTextures(config.textures);
    
    if (config.uniformBuffers) {
        drawable->mutableUniformBuffers() = std::move(*config.uniformBuffers);
    }
    
    // Apply render state
    drawable->setEnableColor(config.enableColor);
    drawable->setColorMode(config.colorMode);
    drawable->setEnableDepth(config.enableDepth);
    drawable->setDepthType(config.depthType);
    drawable->setCullFaceMode(config.cullFaceMode);
    drawable->setLineWidth(static_cast<int32_t>(config.lineWidth));
    
    // Apply draw configuration
    drawable->setDrawModeType(config.drawMode);
    drawable->setVertexOffset(config.vertexOffset);
    drawable->setIndexOffset(config.indexOffset);
    drawable->setInstanceCount(config.instanceCount);
    
    // Apply layer configuration
    drawable->setRenderPass(config.renderPass);
    drawable->setSubLayerIndex(config.subLayerIndex);
    if (config.layerTweaker) {
        drawable->setLayerTweaker(std::move(config.layerTweaker));
    }
    
    // Apply tile configuration
    if (config.tileID) {
        drawable->setTileID(*config.tileID);
    }
    drawable->setSortKey(config.sortKey);
    
    return std::move(drawable);
}

DrawableBuilder& DrawableBuilder::setShader(gfx::ShaderProgramBasePtr shader) {
    config.shader = std::move(shader);
    return *this;
}

DrawableBuilder& DrawableBuilder::setVertexAttributes(gfx::VertexAttributeArrayPtr attributes) {
    config.vertexAttributes = std::move(attributes);
    return *this;
}

DrawableBuilder& DrawableBuilder::setVertexData(std::vector<uint8_t>&& data, std::size_t vertexCount) {
    config.vertexData = std::move(data);
    config.vertexCount = vertexCount;
    return *this;
}

DrawableBuilder& DrawableBuilder::setIndexData(gfx::IndexVectorBasePtr indices, 
                                              std::vector<gfx::Drawable::UniqueDrawSegment> segments) {
    config.indices = std::move(indices);
    config.segments = std::move(segments);
    return *this;
}

DrawableBuilder& DrawableBuilder::setTextures(const gfx::Drawable::Textures& textures) {
    config.textures = textures;
    return *this;
}

DrawableBuilder& DrawableBuilder::setUniformBuffers(gfx::UniformBufferArrayPtr uniformBuffers) {
    config.uniformBuffers = std::move(uniformBuffers);
    return *this;
}

DrawableBuilder& DrawableBuilder::setEnableColor(bool value) {
    config.enableColor = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setColorMode(const gfx::ColorMode& value) {
    config.colorMode = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setEnableDepth(bool value) {
    config.enableDepth = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setDepthType(gfx::DepthMaskType value) {
    config.depthType = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setCullFaceMode(const gfx::CullFaceMode& value) {
    config.cullFaceMode = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setLineWidth(float value) {
    config.lineWidth = value;
    return *this;
}

DrawableBuilder& DrawableBuilder::setDrawMode(gfx::DrawMode mode) {
    config.drawMode = mode;
    return *this;
}

DrawableBuilder& DrawableBuilder::setVertexOffset(std::size_t offset) {
    config.vertexOffset = offset;
    return *this;
}

DrawableBuilder& DrawableBuilder::setIndexOffset(std::size_t offset) {
    config.indexOffset = offset;
    return *this;
}

DrawableBuilder& DrawableBuilder::setInstanceCount(std::size_t count) {
    config.instanceCount = count;
    return *this;
}

DrawableBuilder& DrawableBuilder::setRenderPass(RenderPass pass) {
    config.renderPass = pass;
    return *this;
}

DrawableBuilder& DrawableBuilder::setSubLayerIndex(int32_t index) {
    config.subLayerIndex = index;
    return *this;
}

DrawableBuilder& DrawableBuilder::setLayerTweaker(LayerTweakerPtr tweaker) {
    config.layerTweaker = std::move(tweaker);
    return *this;
}

DrawableBuilder& DrawableBuilder::setTileID(OverscaledTileID tileID) {
    config.tileID = tileID;
    return *this;
}

DrawableBuilder& DrawableBuilder::setSortKey(float key) {
    config.sortKey = key;
    return *this;
}

} // namespace webgpu
} // namespace mbgl