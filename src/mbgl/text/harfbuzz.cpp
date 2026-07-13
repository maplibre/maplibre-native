#include "harfbuzz.hpp"

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
#include "harfbuzz_impl.hpp"
#endif

// TODO: return empty harfbuzz
namespace mbgl {

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
    impl->createComplexGlyphIDs(text, glyphIDs, adjusts);
}

Glyph HBShaper::rasterizeGlyph(const GlyphID &glyph) {
    return impl->rasterizeGlyph(glyph);
}

bool HBShaper::valid() {
    return impl->valid();
}

} // namespace mbgl
