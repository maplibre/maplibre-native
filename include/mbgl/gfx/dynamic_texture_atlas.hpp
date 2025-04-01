#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/style/image_impl.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

namespace gfx {

class Context;
class DynamicTexture;
using DynamicTexturePtr = std::shared_ptr<gfx::DynamicTexture>;
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::unique_ptr<gfx::DynamicTextureAtlas>;

class TexturePackHandle {
public:
    TexturePackHandle() = default;
    ~TexturePackHandle() = default;

    const std::vector<mapbox::Bin*>& getBins() const { return bins; }
    const DynamicTexturePtr& getDynamicTexture() const { return dynamicTexture; }
        
private:
    std::vector<mapbox::Bin*> bins;
    DynamicTexturePtr dynamicTexture;
    
    friend class DynamicTextureAtlas;
};

class GlyphTexturePack {
public:
    GlyphPositions positions;
    TexturePackHandle handle;
};

class ImageTexturePack {
public:
    ImagePositions iconPositions;
    ImagePositions patternPositions;
    TexturePackHandle handle;
};

class DynamicTextureAtlas {
public:
    DynamicTextureAtlas(Context& context_)
        : context(context_) {}
    ~DynamicTextureAtlas() = default;
    
    GlyphTexturePack uploadGlyphs(const GlyphMap& glyphs);
    ImageTexturePack uploadIconsAndPatterns(const ImageMap& icons, const ImageMap& patterns, const ImageVersionMap& versionMap);
    void uploadDeferredImages();
    
private:
    Context& context;
    std::vector<DynamicTexturePtr> dynamicTextures;
    std::mutex mutex;
};

#define MLN_DEFER_UPLOAD_ON_RENDER_THREAD (MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN)

} // namespace gfx
} // namespace mbgl
