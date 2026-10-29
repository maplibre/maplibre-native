#pragma once

#include <mln/gfx/dynamic_texture.hpp>
#include <mln/text/glyph.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/util/hash.hpp>

#include <unordered_map>

namespace mln {

namespace gfx {

using DynamicTexturePtr = std::shared_ptr<gfx::DynamicTexture>;

class GlyphAtlas {
public:
    GlyphPositions glyphPositions;
    std::vector<TextureHandle> textureHandles;
    DynamicTexturePtr dynamicTexture;

    int32_t generateUniqueId(const FontStackHash& fontStack, const GlyphID& glyphId) const;

    void uploadGlyph(const Immutable<Glyph>& glyph, TextureHandle& texHandle);
};

class ImageAtlas {
public:
    ImagePositions iconPositions;
    ImagePositions patternPositions;
    std::vector<TextureHandle> textureHandles;
    DynamicTexturePtr dynamicTexture;

    int32_t generateUniqueId(const std::string& imageId, Size size) const;

    void uploadIcon(const Immutable<style::Image::Impl>& glyph, TextureHandle& texHandle);
    void uploadPattern(const Immutable<style::Image::Impl>& glyph, TextureHandle& texHandle);
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

    void removeTextures(const std::vector<TextureHandle>& textureHandles, const DynamicTexturePtr& dynamicTexture);
    void removeUnusedDynamicTextures();

private:
    Context& context;
    std::vector<DynamicTexturePtr> dynamicTextures;
    std::unordered_map<TexturePixelType, DynamicTexturePtr> dummyDynamicTexture;
    std::mutex mutex;
};

} // namespace gfx
} // namespace mln
