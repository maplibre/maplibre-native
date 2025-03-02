#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/gfx/context.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

GlyphPositions uploadGlyphs(const GlyphMap& glyphs) {
    GlyphPositions glyphPositions;

    for (const auto& glyphMapEntry : glyphs) {
        FontStackHash fontStack = glyphMapEntry.first;
        GlyphPositionMap& positions = glyphPositions[fontStack];

        for (const auto& entry : glyphMapEntry.second) {
            if (entry.second && (*entry.second)->bitmap.valid()) {
                const Glyph& glyph = **entry.second;

                int32_t uniqueId = static_cast<int32_t>(sqrt(fontStack) / 2 + glyph.id);
                auto glyphHandle = gfx::Context::getDynamicTexture()->addImage(glyph.bitmap, uniqueId);

                if (glyphHandle) {
                    positions.emplace(glyph.id, GlyphPosition{*glyphHandle, glyph.metrics});
                }
            }
        }
    }
    return glyphPositions;
}

} // namespace mbgl
