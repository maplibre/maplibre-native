#pragma once

#include "freetype.hpp"
#include "harfbuzz.hpp"
#include <map>

struct hb_font_t;
struct hb_buffer_t;

using hb_font_t = hb_font_t;
using hb_buffer_t = hb_buffer_t;

namespace mbgl {

class HBShaper::Impl {
public:
    explicit Impl(GlyphIDType type_, const std::string &fontFileData, const FreeTypeLibrary &lib);
    ~Impl();

    void createComplexGlyphIDs(const std::u16string &text,
                               std::vector<GlyphID> &glyphIDs,
                               std::vector<HBShapeAdjust> &adjusts);

    Glyph rasterizeGlyph(const GlyphID &glyph) { return face.rasterizeGlyph(glyph); }

    bool valid() { return face.isValid(); }

private:
    GlyphIDType type;
    FreeTypeFace face;

    hb_font_t *font;
    hb_buffer_t *buffer;
};

} // namespace mbgl
