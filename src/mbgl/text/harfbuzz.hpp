#pragma once

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
#include <harfbuzz_wrap.hpp>
#else
struct FreeTypeLibrary {};
struct HBShapeAdjust {
    float x_offset;
    float y_offset;

    float advance; // advanceX

    HBShapeAdjust(float x, float y, float a)
        : x_offset(x),
          y_offset(y),
          advance(a) {}
};
#endif
#include "glyph.hpp"

namespace mbgl {

class HBShaper {
public:
    explicit HBShaper(GlyphIDType type_, const std::string &fontFileData, const FreeTypeLibrary &lib);
    ~HBShaper();

    void CreateComplexGlyphIDs(const std::u16string &text,
                               std::vector<GlyphID> &glyphIDs,
                               std::vector<HBShapeAdjust> &adjusts);
    Glyph rasterizeGlyph(GlyphID glyphID);

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    bool Valid() { return shaper->Valid(); }

private:
    GlyphIDType type;
    std::unique_ptr<HBShaperWrap> shaper = nullptr;
#else
    bool Valid() { return false; }
#endif
};

} // namespace mbgl
