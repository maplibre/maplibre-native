#include "harfbuzz.hpp"

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
#include "harfbuzz_impl.hpp"
#endif

// TODO: return empty harfbuzz
namespace mbgl {

#define SDF_FONT_SIZE 24

#ifndef MLN_TEXT_SHAPING_HARFBUZZ

class HBShaper::Impl {
public:
    Impl(GlyphIDType, const std::string &, const FreeTypeLibrary &) {}

    bool valid() { return false; }

    void createComplexGlyphIDs(const std::u16string &, std::vector<GlyphID> &, std::vector<HBShapeAdjust> &) {
        assert(false && "can't shaping text without harfbuzz.");
    }

    Glyph rasterizeGlyph(const GlyphID &) {
        assert(false && "can't rasterize glyph without harfbuzz + freetype.");
        return {};
    }
};

#endif

HBShaper::HBShaper(GlyphIDType type, const std::string &fontFileData, const FreeTypeLibrary &lib) {
    impl = std::make_unique<Impl>(type, fontFileData, lib);
}

HBShaper::~HBShaper() {
    impl.reset();
}

void HBShaper::createComplexGlyphIDs(const std::u16string &text,
                                     std::vector<GlyphID> &glyphIDs,
                                     std::vector<HBShapeAdjust> &adjusts) {
    return impl->createComplexGlyphIDs(text, glyphIDs, adjusts);

    // std::vector<uint32_t> indexs;
    // impl->createComplexGlyphIDs(text, indexs, adjusts);
    // for (auto index : indexs) {
    //     glyphIDs.emplace_back((char16_t)index, type);
    // }
}

Glyph HBShaper::rasterizeGlyph(const GlyphID &glyph) {
    return impl->rasterizeGlyph(glyph);

    // Glyph fixedMetrics;

    // auto setGlyph = [&](uint32_t width, uint32_t height, int left, int top, uint32_t advance, unsigned char *buffer)
    // {
    //     fixedMetrics.id = glyphID;

    //     Size size(width + Glyph::borderSize * 2, height + Glyph::borderSize * 2);

    //     fixedMetrics.metrics.width = size.width;
    //     fixedMetrics.metrics.height = size.height;
    //     fixedMetrics.metrics.left = left;
    //     fixedMetrics.metrics.top = top - SDF_FONT_SIZE - Glyph::borderSize;
    //     fixedMetrics.metrics.advance = advance;

    //     // Copy alpha values from RGBA bitmap into the AlphaImage output
    //     fixedMetrics.bitmap = AlphaImage(size);

    //     for (uint32_t h = 0; h < height; ++h) {
    //         std::memcpy(&fixedMetrics.bitmap.data[size.width * (h + Glyph::borderSize) + Glyph::borderSize],
    //                     &buffer[width * h],
    //                     width);
    //     }
    // };

    // impl->rasterizeGlyph(glyphID.complex.code, setGlyph);

    // return fixedMetrics;
}

bool HBShaper::valid() {
    return impl->valid();
}

} // namespace mbgl
