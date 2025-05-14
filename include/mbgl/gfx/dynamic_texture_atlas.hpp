#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/text/glyph.hpp>
#include <mbgl/style/image_impl.hpp>

namespace mbgl {

namespace gfx {

using DynamicTexturePtr = std::shared_ptr<gfx::DynamicTexture>;

class GlyphAtlas {
public:
    GlyphPositions glyphPositions;
    std::vector<TextureHandle> textureHandles;
    DynamicTexturePtr dynamicTexture;
};

class ImageAtlas {
public:
    ImagePositions iconPositions;
    ImagePositions patternPositions;
    std::vector<TextureHandle> textureHandles;
    DynamicTexturePtr dynamicTexture;
};

class DynamicTextureAtlas {
public:
    DynamicTextureAtlas(Context& context_)
        : context(context_) {}
    ~DynamicTextureAtlas() = default;

    GlyphAtlas uploadGlyphs(const GlyphMap& glyphs,
                            std::vector<std::function<void(Context&)>>& deletionQueue,
                            const vk::UniqueCommandBuffer& commandBuffer);
    ImageAtlas uploadIconsAndPatterns(const ImageMap& icons,
                                      const ImageMap& patterns,
                                      const ImageVersionMap& versionMap,
                                      std::vector<std::function<void(Context&)>>& deletionQueue,
                                      const vk::UniqueCommandBuffer& commandBuffer);

    void removeTextures(const std::vector<TextureHandle>& textureHandles, const DynamicTexturePtr& dynamicTexture);

    Context& context;
private:
    std::vector<DynamicTexturePtr> dynamicTextures;
    std::unordered_map<TexturePixelType, DynamicTexturePtr> dummyDynamicTexture;
    std::mutex mutex;
};

} // namespace gfx
} // namespace mbgl
