#include "harfbuzz.hpp"

namespace mbgl {

#define SDF_FONT_SIZE 24

HBShaper::HBShaper(GlyphIDType type_, const std::string &fontFileData, const FreeTypeLibrary &lib)
    : type(type_) {
    shaper = std::make_unique<HBShaperWrap>((GlyphTypeWrap)type_, fontFileData, lib);

    if (!shaper || !shaper->Valid()) return;
}

HBShaper::~HBShaper() {
    shaper.reset();
}


void HBShaper::CreateComplexGlyphIDs(const std::u16string &text,
                                     std::vector<GlyphID> &glyphIDs,
                                     std::vector<HBShapeAdjust> &adjusts) {
    std::vector<uint32_t> indexs;
    shaper->CreateComplexGlyphIDs(text, indexs, adjusts);
    for (auto index : indexs) {
        glyphIDs.emplace_back((char16_t)index, type);
    }
}

Glyph HBShaper::rasterizeGlyph(GlyphID glyphID) {
    Glyph fixedMetrics;

    auto setGlyph = [&](uint32_t width, uint32_t height, int left, int top, uint32_t advance, unsigned char *buffer) {
        fixedMetrics.id = glyphID;

        Size size(width + Glyph::borderSize * 2, height + Glyph::borderSize * 2);

        fixedMetrics.metrics.width = size.width;
        fixedMetrics.metrics.height = size.height;
        fixedMetrics.metrics.left = left;
        fixedMetrics.metrics.top = top - SDF_FONT_SIZE - Glyph::borderSize;
        fixedMetrics.metrics.advance = advance;

        // Copy alpha values from RGBA bitmap into the AlphaImage output
        fixedMetrics.bitmap = AlphaImage(size);

        for (uint32_t h = 0; h < height; ++h) {
            std::memcpy(&fixedMetrics.bitmap.data[size.width * (h + Glyph::borderSize) + Glyph::borderSize],
                        &buffer[width * h],
                        width);
        }
    };

    shaper->rasterizeGlyph(glyphID.complex.code, setGlyph);

    return fixedMetrics;
}

} // namespace mbgl
