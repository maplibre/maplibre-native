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

    GlyphAtlas uploadGlyphs(const GlyphMap& glyphs);
    ImageAtlas uploadIconsAndPatterns(const ImageMap& icons,
                                      const ImageMap& patterns,
                                      const ImageVersionMap& versionMap);
    void uploadDeferredImages();

private:
    Context& context;
    std::vector<DynamicTexturePtr> dynamicTextures;
    std::mutex mutex;
};

#define MLN_DEFER_UPLOAD_ON_RENDER_THREAD (MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN)

} // namespace gfx
} // namespace mbgl
