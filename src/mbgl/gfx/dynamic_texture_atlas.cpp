#include <mbgl/gfx/dynamic_texture_atlas.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

GlyphAtlas DynamicTextureAtlas::uploadGlyphs(const GlyphMap& glyphs) {
    GlyphAtlas glyphAtlas;

    mutex.lock();
    for (const auto& dynamicTexture : dynamicTextures) {
        if (dynamicTexture->getPixelFormat() != TexturePixelType::Alpha) {
            continue;
        }
        bool hasSpace = true;
        for (const auto& glyphMapEntry : glyphs) {
            FontStackHash fontStack = glyphMapEntry.first;

            for (const auto& glyphEntry : glyphMapEntry.second) {
                if (glyphEntry.second && (*glyphEntry.second)->bitmap.valid()) {
                    const Glyph& glyph = **glyphEntry.second;

                    int32_t uniqueId = static_cast<int32_t>(sqrt(fontStack) / 2 + glyph.id);
                    const auto& glyphHandle = dynamicTexture->addImage(glyph.bitmap, uniqueId);
                    if (!glyphHandle) {
                        hasSpace = false;
                        break;
                    }
                    glyphAtlas.textureHandles.emplace_back(*glyphHandle);
                }
            }
            if (!hasSpace) {
                for (const auto& texHandle : glyphAtlas.textureHandles) {
                    dynamicTexture->removeTexture(texHandle);
                }
                glyphAtlas.textureHandles.clear();
                break;
            }
        }
        if (hasSpace) {
            glyphAtlas.dynamicTexture = dynamicTexture;
            break;
        }
    }

    if (!glyphAtlas.dynamicTexture) {
        glyphAtlas.dynamicTexture = std::make_shared<gfx::DynamicTexture>(
            context, Size{2048, 2048}, TexturePixelType::Alpha);
        dynamicTextures.emplace_back(glyphAtlas.dynamicTexture);
    }

    for (const auto& glyphMapEntry : glyphs) {
        FontStackHash fontStack = glyphMapEntry.first;
        GlyphPositionMap& positions = glyphAtlas.glyphPositions[fontStack];

        for (const auto& entry : glyphMapEntry.second) {
            if (entry.second && (*entry.second)->bitmap.valid()) {
                const Glyph& glyph = **entry.second;

                int32_t uniqueId = static_cast<int32_t>(sqrt(fontStack) / 2 + glyph.id);
                const auto& glyphHandle = glyphAtlas.dynamicTexture->addImage(glyph.bitmap, uniqueId);
                assert(glyphHandle.has_value());

                if (glyphHandle.has_value()) {
                    glyphAtlas.textureHandles.emplace_back(*glyphHandle);
                    positions.emplace(glyph.id,
                                      GlyphPosition{glyphHandle->getRectangle(), glyph.metrics});
                }
            }
        }
    }
    mutex.unlock();
    return glyphAtlas;
}

ImageAtlas DynamicTextureAtlas::uploadIconsAndPatterns(const ImageMap& icons,
                                                       const ImageMap& patterns,
                                                       const ImageVersionMap& versionMap) {
    ImageAtlas imageAtlas;

    mutex.lock();
    for (const auto& dynamicTexture : dynamicTextures) {
        if (dynamicTexture->getPixelFormat() != TexturePixelType::RGBA) {
            continue;
        }
        bool hasSpace = true;
        for (const auto& iconEntry : icons) {
            const style::Image::Impl& icon = *iconEntry.second;

            auto imageHash = util::hash(icon.id);
            int32_t uniqueId = static_cast<int32_t>(sqrt(imageHash) / 2);
            const auto& iconHandle = dynamicTexture->addImage(icon.image, uniqueId);
            if (!iconHandle) {
                hasSpace = false;
                break;
            }
            imageAtlas.textureHandles.emplace_back(*iconHandle);
        }
        if (!hasSpace) {
            for (const auto& texHandle : imageAtlas.textureHandles) {
                dynamicTexture->removeTexture(texHandle);
            }
            imageAtlas.textureHandles.clear();
            continue;
        }
        for (const auto& patternEntry : patterns) {
            const style::Image::Impl& pattern = *patternEntry.second;

            auto patternHash = util::hash(pattern.id);
            int32_t uniqueId = static_cast<int32_t>(sqrt(patternHash) / 2);
            const auto& patternHandle = dynamicTexture->addImage(pattern.image, uniqueId);
            if (!patternHandle) {
                hasSpace = false;
                break;
            }
            imageAtlas.textureHandles.emplace_back(*patternHandle);
        }
        if (!hasSpace) {
            for (const auto& texHandle : imageAtlas.textureHandles) {
                dynamicTexture->removeTexture(texHandle);
            }
            imageAtlas.textureHandles.clear();
            continue;
        }
        if (hasSpace) {
            imageAtlas.dynamicTexture = dynamicTexture;
            break;
        }
    }

    if (!imageAtlas.dynamicTexture) {
        imageAtlas.dynamicTexture = std::make_shared<gfx::DynamicTexture>(
            context, Size{1024, 1024}, TexturePixelType::RGBA);
        dynamicTextures.emplace_back(imageAtlas.dynamicTexture);
    }

    imageAtlas.iconPositions.reserve(icons.size());
    for (const auto& iconEntry : icons) {
        const style::Image::Impl& icon = *iconEntry.second;

        auto imageHash = util::hash(icon.id);
        int32_t uniqueId = static_cast<int32_t>(sqrt(imageHash) / 2);
        const auto& iconHandle = imageAtlas.dynamicTexture->addImage(icon.image, uniqueId);
        assert(iconHandle.has_value());

        if (iconHandle.has_value()) {
            const auto it = versionMap.find(iconEntry.first);
            const auto version = it != versionMap.end() ? it->second : 0;
            imageAtlas.iconPositions.emplace(icon.id, ImagePosition{iconHandle->getRectangle(), icon, version});
            imageAtlas.textureHandles.emplace_back(*iconHandle);
        }
    }

    for (const auto& patternEntry : patterns) {
        const style::Image::Impl& pattern = *patternEntry.second;

        auto patternHash = util::hash(pattern.id);
        int32_t uniqueId = static_cast<int32_t>(sqrt(patternHash) / 2);
        const auto& patternHandle = imageAtlas.dynamicTexture->addImage(pattern.image, uniqueId);
        assert(patternHandle.has_value());

        if (patternHandle.has_value()) {
            const auto it = versionMap.find(patternEntry.first);
            const auto version = it != versionMap.end() ? it->second : 0;
            imageAtlas.patternPositions.emplace(pattern.id,
                                                  ImagePosition{patternHandle->getRectangle(), pattern, version});
            imageAtlas.textureHandles.emplace_back(*patternHandle);
        }
    }

    mutex.unlock();
    return imageAtlas;
}

void DynamicTextureAtlas::uploadDeferredImages() {
    for (const auto& dynamicTexture : dynamicTextures) {
        dynamicTexture->uploadDeferredImages();
    }
}

} // namespace gfx
} // namespace mbgl
