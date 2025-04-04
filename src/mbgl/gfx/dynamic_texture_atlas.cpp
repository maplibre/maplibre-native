#include <mbgl/gfx/dynamic_texture_atlas.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

GlyphAtlas DynamicTextureAtlas::uploadGlyphs(const GlyphMap& glyphs) {
    using GlyphsToUpload = std::vector<std::tuple<TextureHandle, Immutable<Glyph>, FontStackHash>>;
    mutex.lock();

    size_t dynTexIndex = 0;
    Size dynTexSize = {512, 512};
    GlyphsToUpload glyphsToUpload;
    GlyphAtlas glyphAtlas;
    
    while (!glyphAtlas.dynamicTexture) {
        if (dynTexIndex < dynamicTextures.size()) {
            glyphAtlas.dynamicTexture = dynamicTextures[dynTexIndex++];
        } else {
            glyphAtlas.dynamicTexture = std::make_shared<gfx::DynamicTexture>(context, dynTexSize, TexturePixelType::Alpha);
            dynTexSize = Size(dynTexSize.width * 2, dynTexSize.height * 2);
        }
        
        if (glyphAtlas.dynamicTexture->getPixelFormat() != TexturePixelType::Alpha) {
            glyphAtlas.dynamicTexture = nullptr;
            continue;
        }
        
        bool hasSpace = true;
        for (const auto& glyphMapEntry : glyphs) {
            FontStackHash fontStack = glyphMapEntry.first;

            for (const auto& glyphEntry : glyphMapEntry.second) {
                const auto& glyph = glyphEntry.second;
                
                if (glyph.has_value() && glyph.value()->bitmap.valid()) {
                    int32_t uniqueId = static_cast<int32_t>(sqrt(fontStack) / 2 + glyph.value()->id);
                    const auto& texHandle = glyphAtlas.dynamicTexture->reserveSize(glyph.value()->bitmap.size, uniqueId);
                    if (!texHandle) {
                        hasSpace = false;
                        break;
                    }
                    glyphsToUpload.emplace_back(std::make_tuple(*texHandle, glyph.value(), fontStack));
                }
            }
            if (!hasSpace) {
                glyphsToUpload.clear();
                glyphAtlas.dynamicTexture = nullptr;
                break;
            }
        }
    }
    if (dynTexIndex == dynamicTextures.size()) {
        dynamicTextures.emplace_back(glyphAtlas.dynamicTexture);
    }

    for (const auto& tuple : glyphsToUpload) {
        const auto& texHandle = std::get<0>(tuple);
        const auto& glyph = std::get<1>(tuple);
        auto fontStack = std::get<2>(tuple);
        
        glyphAtlas.dynamicTexture->uploadImage(glyph->bitmap.data.get(), texHandle);
        glyphAtlas.textureHandles.emplace_back(texHandle);
        glyphAtlas.glyphPositions[fontStack].emplace(glyph->id,
                                                     GlyphPosition{texHandle.getRectangle(), glyph->metrics});
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
    mutex.lock();
    for (const auto& dynamicTexture : dynamicTextures) {
        dynamicTexture->uploadDeferredImages();
    }
    mutex.unlock();
}

void DynamicTextureAtlas::removeTextures(const std::vector<TextureHandle>& textureHandles,
                                         const DynamicTexturePtr& dynamicTexture) {
    if (!dynamicTexture) {
        return;
    }
    mutex.lock();
    for (const auto& texHandle : textureHandles) {
        dynamicTexture->removeTexture(texHandle);
    }
    if (dynamicTexture->isEmpty()) {
        auto iterator = std::find(dynamicTextures.begin(), dynamicTextures.end(), dynamicTexture);
        if(iterator != dynamicTextures.end()) {
            dynamicTextures.erase(iterator);
        }
    }
    mutex.unlock();
}

} // namespace gfx
} // namespace mbgl
