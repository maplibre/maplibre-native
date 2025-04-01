#include <mbgl/gfx/dynamic_texture_atlas.hpp>
#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

GlyphTexturePack DynamicTextureAtlas::uploadGlyphs(const GlyphMap& glyphs) {
    GlyphTexturePack glyphTexPack;

    mutex.lock();
    for (const auto& dynamicTexture: dynamicTextures) {
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
                    auto glyphHandle = dynamicTexture->addImage(glyph.bitmap, uniqueId);
                    if (!glyphHandle) {
                        hasSpace = false;
                        break;
                    }
                    glyphTexPack.handle.bins.emplace_back(glyphHandle->getBin());
                }
            }
            if (!hasSpace) {
                for (const auto& bin : glyphTexPack.handle.bins) {
                    dynamicTexture->removeTexture(TextureHandle(bin));
                }
                glyphTexPack.handle.bins.clear();
                break;
            }
        }
        if (hasSpace) {
            glyphTexPack.handle.dynamicTexture = dynamicTexture;
            break;
        }
    }

    if (!glyphTexPack.handle.dynamicTexture) {
        glyphTexPack.handle.dynamicTexture = std::make_shared<gfx::DynamicTexture>(context, Size{2048, 2048}, TexturePixelType::Alpha);
        dynamicTextures.emplace_back(glyphTexPack.handle.dynamicTexture);
    }
    
    for (const auto& glyphMapEntry : glyphs) {
        FontStackHash fontStack = glyphMapEntry.first;
        GlyphPositionMap& positions = glyphTexPack.positions[fontStack];

        for (const auto& entry : glyphMapEntry.second) {
            if (entry.second && (*entry.second)->bitmap.valid()) {
                const Glyph& glyph = **entry.second;

                int32_t uniqueId = static_cast<int32_t>(sqrt(fontStack) / 2 + glyph.id);
                auto glyphHandle = glyphTexPack.handle.dynamicTexture->addImage(glyph.bitmap, uniqueId);
                assert(glyphHandle.has_value());
                
                if (glyphHandle.has_value()) {
                    const auto& bin = glyphHandle->getBin();
                    glyphTexPack.handle.bins.emplace_back(bin);
                    positions.emplace(glyph.id, GlyphPosition{Rect<uint16_t>(bin->x, bin->y, bin->w, bin->h), glyph.metrics});
                }
            }
        }
    }
    mutex.unlock();
    return glyphTexPack;
}

ImageTexturePack DynamicTextureAtlas::uploadIconsAndPatterns(const ImageMap& icons, const ImageMap& patterns, const ImageVersionMap& versionMap) {
    ImageTexturePack imageTexPack;
    
    mutex.lock();
    for (const auto& dynamicTexture: dynamicTextures) {
        if (dynamicTexture->getPixelFormat() != TexturePixelType::RGBA) {
            continue;
        }
        bool hasSpace = true;
        for (const auto& iconEntry : icons) {
            const style::Image::Impl& icon = *iconEntry.second;
            
            auto imageHash = util::hash(icon.id);
            int32_t uniqueId = static_cast<int32_t>(sqrt(imageHash) / 2);
            auto iconHandle = dynamicTexture->addImage(icon.image, uniqueId);
            if (!iconHandle) {
                hasSpace = false;
                break;
            }
            imageTexPack.handle.bins.emplace_back(iconHandle->getBin());
        }
        if (!hasSpace) {
            for (const auto& bin : imageTexPack.handle.bins) {
                dynamicTexture->removeTexture(TextureHandle(bin));
            }
            imageTexPack.handle.bins.clear();
            continue;
        }
        for (const auto& patternEntry : patterns) {
            const style::Image::Impl& pattern = *patternEntry.second;
            
            auto patternHash = util::hash(pattern.id);
            int32_t uniqueId = static_cast<int32_t>(sqrt(patternHash) / 2);
            auto patternHandle = dynamicTexture->addImage(pattern.image, uniqueId);
            if (!patternHandle) {
                hasSpace = false;
                break;
            }
            imageTexPack.handle.bins.emplace_back(patternHandle->getBin());
        }
        if (!hasSpace) {
            for (const auto& bin : imageTexPack.handle.bins) {
                dynamicTexture->removeTexture(TextureHandle(bin));
            }
            imageTexPack.handle.bins.clear();
            continue;
        }
        if (hasSpace) {
            imageTexPack.handle.dynamicTexture = dynamicTexture;
            break;
        }
    }

    if (!imageTexPack.handle.dynamicTexture) {
        imageTexPack.handle.dynamicTexture = std::make_shared<gfx::DynamicTexture>(context, Size{1024, 1024}, TexturePixelType::RGBA);
        dynamicTextures.emplace_back(imageTexPack.handle.dynamicTexture);
    }
    
    imageTexPack.iconPositions.reserve(icons.size());
    for (const auto& iconEntry : icons) {
        const style::Image::Impl& icon = *iconEntry.second;
        
        auto imageHash = util::hash(icon.id);
        int32_t uniqueId = static_cast<int32_t>(sqrt(imageHash) / 2);
        auto iconHandle = imageTexPack.handle.dynamicTexture->addImage(icon.image, uniqueId);
        assert(iconHandle.has_value());
        
        if (iconHandle.has_value()) {
            const auto it = versionMap.find(iconEntry.first);
            const auto version = it != versionMap.end() ? it->second : 0;
            imageTexPack.iconPositions.emplace(icon.id, ImagePosition{*iconHandle->getBin(), icon, version});
            imageTexPack.handle.bins.emplace_back(iconHandle->getBin());
        }
    }
   
    for (const auto& patternEntry : patterns) {
        const style::Image::Impl& pattern = *patternEntry.second;
        
        auto patternHash = util::hash(pattern.id);
        int32_t uniqueId = static_cast<int32_t>(sqrt(patternHash) / 2);
        auto patternHandle = imageTexPack.handle.dynamicTexture->addImage(pattern.image, uniqueId);
        assert(patternHandle.has_value());
        
        if (patternHandle.has_value()) {
            const auto it = versionMap.find(patternEntry.first);
            const auto version = it != versionMap.end() ? it->second : 0;
            imageTexPack.patternPositions.emplace(pattern.id, ImagePosition{*patternHandle->getBin(), pattern, version});
            imageTexPack.handle.bins.emplace_back(patternHandle->getBin());
        }
    }

    mutex.unlock();
    return imageTexPack;
}

void DynamicTextureAtlas::uploadDeferredImages() {
    for(const auto& dynamicTexture : dynamicTextures) {
        dynamicTexture->uploadDeferredImages();
    }
}

} // namespace gfx
} // namespace mbgl
