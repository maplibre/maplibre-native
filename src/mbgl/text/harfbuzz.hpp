#pragma once

#include "freetype.hpp"

struct hb_font_t;
struct hb_buffer_t;

using hb_font_t = hb_font_t;
using hb_buffer_t = hb_buffer_t;

namespace mbgl {

struct HBShapeAdjust {
    float x_offset;
    float y_offset;

    float advance; // advanceX

    HBShapeAdjust(float x, float y, float a)
        : x_offset(x),
          y_offset(y),
          advance(a) {}
};

struct InternalHBLangInfo;

class HBShaper {
public:
    explicit HBShaper(GlyphIDType _type, const std::string &fontFileData, const FreeTypeLibrary &lib);
    ~HBShaper();

    void CreateComplexGlyphIDs(const std::string &text,
                               std::vector<GlyphID> &glyphIDs,
                               std::vector<HBShapeAdjust> &adjusts);
    void CreateComplexGlyphIDs(const std::u16string &text,
                               std::vector<GlyphID> &glyphIDs,
                               std::vector<HBShapeAdjust> &adjusts);
    Glyph rasterizeGlyph(GlyphID glyphID) { return face.rasterizeGlyph(glyphID); }

    bool Valid() { return face.Valid(); }

private:
    // Load freetype font from
    explicit HBShaper(GlyphIDType _type, const FreeTypeLibrary &lib);
    FreeTypeFace face;

    hb_font_t *font;
    hb_buffer_t *buffer;
    InternalHBLangInfo *hbInfo;
    GlyphIDType type;

    static std::map<GlyphIDType, InternalHBLangInfo> internalInfos;
};

} // namespace mbgl
