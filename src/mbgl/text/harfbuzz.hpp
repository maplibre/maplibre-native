#pragma once

#include <mbgl/text/glyph.hpp>

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
#include "freetype.hpp"
#endif

namespace mbgl {

#ifndef MLN_TEXT_SHAPING_HARFBUZZ
struct FreeTypeLibrary {};
#endif

struct HBShapeAdjust {
    float x_offset;
    float y_offset;

    float advance; // advanceX

    HBShapeAdjust(float x, float y, float a)
        : x_offset(x),
          y_offset(y),
          advance(a) {}
};

class HBShaper {
public:
    explicit HBShaper(GlyphIDType type, const std::string &fontFileData, const FreeTypeLibrary &lib);
    ~HBShaper();

    void createComplexGlyphIDs(const std::u16string &text,
                               std::vector<GlyphID> &glyphIDs,
                               std::vector<HBShapeAdjust> &adjusts);
    Glyph rasterizeGlyph(const GlyphID &glyph);

    bool valid();

private:
    class Impl;

    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
