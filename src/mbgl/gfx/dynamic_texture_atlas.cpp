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
                for (const auto& tuple : glyphsToUpload) {
                    const auto& texHandle = std::get<0>(tuple);
                    glyphAtlas.dynamicTexture->removeTexture(texHandle);
                }
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
    using ImagesToUpload = std::vector<std::pair<TextureHandle, Immutable<style::Image::Impl>>>;
    mutex.lock();

    size_t dynTexIndex = 0;
    Size dynTexSize = {512, 512};
    ImagesToUpload iconsToUpload;
    ImagesToUpload patternsToUpload;
    ImageAtlas imageAtlas;
    
    while (!imageAtlas.dynamicTexture) {
        if (dynTexIndex < dynamicTextures.size()) {
            imageAtlas.dynamicTexture = dynamicTextures[dynTexIndex++];
        } else {
            imageAtlas.dynamicTexture = std::make_shared<gfx::DynamicTexture>(context, dynTexSize, TexturePixelType::RGBA);
            dynTexSize = Size(dynTexSize.width * 2, dynTexSize.height * 2);
        }
        
        if (imageAtlas.dynamicTexture->getPixelFormat() != TexturePixelType::RGBA) {
            imageAtlas.dynamicTexture = nullptr;
            continue;
        }

        bool hasSpace = true;
        for (const auto& iconEntry : icons) {
            const auto& icon = iconEntry.second;

            auto imageHash = util::hash(icon->id);
            int32_t uniqueId = static_cast<int32_t>(sqrt(imageHash) / 2);
            const auto& texHandle = imageAtlas.dynamicTexture->reserveSize(icon->image.size, uniqueId);
            if (!texHandle) {
                hasSpace = false;
                break;
            }
            iconsToUpload.emplace_back(std::make_tuple(*texHandle, icon));
        }
        if (hasSpace) {
            for (const auto& patternEntry : patterns) {
                const auto& pattern = patternEntry.second;
                
                auto patternHash = util::hash(pattern->id);
                int32_t uniqueId = static_cast<int32_t>(sqrt(patternHash) / 2);
                const auto& texHandle = imageAtlas.dynamicTexture->reserveSize(pattern->image.size, uniqueId);
                if (!texHandle) {
                    hasSpace = false;
                    break;
                }
                patternsToUpload.emplace_back(std::make_tuple(*texHandle, pattern));
            }
        }
        if (!hasSpace) {
            for (const auto& pair : iconsToUpload) {
                const auto& texHandle = pair.first;
                imageAtlas.dynamicTexture->removeTexture(texHandle);
            }
            iconsToUpload.clear();
            imageAtlas.dynamicTexture = nullptr;
            continue;
        }
    }
    if (dynTexIndex == dynamicTextures.size()) {
        dynamicTextures.emplace_back(imageAtlas.dynamicTexture);
    }
    
    imageAtlas.iconPositions.reserve(icons.size());
    for (const auto& pair : iconsToUpload) {
        const auto& texHandle = pair.first;
        const auto& icon = pair.second;
        
        imageAtlas.dynamicTexture->uploadImage(icon->image.data.get(), texHandle);
        imageAtlas.textureHandles.emplace_back(texHandle);
        const auto it = versionMap.find(icon->id);
        const auto version = it != versionMap.end() ? it->second : 0;
        imageAtlas.iconPositions.emplace(icon->id, ImagePosition{texHandle.getRectangle(), *icon, version});
    }
    
    imageAtlas.patternPositions.reserve(patterns.size());
    for (const auto& pair : patternsToUpload) {
        const auto& texHandle = pair.first;
        const auto& pattern = pair.second;
        
        imageAtlas.dynamicTexture->uploadImage(pattern->image.data.get(), texHandle);
        imageAtlas.textureHandles.emplace_back(texHandle);
        const auto it = versionMap.find(pattern->id);
        const auto version = it != versionMap.end() ? it->second : 0;
        imageAtlas.patternPositions.emplace(pattern->id, ImagePosition{texHandle.getRectangle(), *pattern, version});
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
