#pragma once

#include "freetype_wrap.hpp"
#include <map>

struct hb_font_t;
struct hb_buffer_t;

using hb_font_t = hb_font_t;
using hb_buffer_t = hb_buffer_t;

namespace mbgl {

struct InternalHBLangInfo;

enum GlyphTypeWrap : short {
    FontPBFWrap = 0x00,
    KhmerWrap = 0x01,
    MyanmarWrap = 0x02,
    DevanagariWrap = 0x03,
};

struct HBShapeAdjust {
    float x_offset;
    float y_offset;
    
    float advance; // advanceX
    
    HBShapeAdjust(float x, float y, float a) : x_offset(x), y_offset(y), advance(a) {}
};

class HBShaperWrap {
public:
    explicit HBShaperWrap(GlyphTypeWrap glyphType, const std::string& fontFileData, const FreeTypeLibrary& lib);
    ~HBShaperWrap();

    void CreateComplexGlyphIDs(const std::string& text,
                               std::vector<uint32_t>& glyphIndexs,
                               std::vector<HBShapeAdjust>& adjusts);
    void CreateComplexGlyphIDs(const std::u16string& text,
                               std::vector<uint32_t>& glyphIndexs,
                               std::vector<HBShapeAdjust>& adjusts);
    
    void rasterizeGlyph(char16_t glyph, GlyphCallBack callback) {  face.rasterizeGlyph(glyph, callback); }
    
    bool Valid() { return face.Valid(); }

private:
    FreeTypeFaceWrap face;

    hb_font_t *font;
    hb_buffer_t *buffer;
    
    InternalHBLangInfo *hbInfo;
    GlyphTypeWrap type;
    
    static std::map<GlyphTypeWrap, InternalHBLangInfo> internalInfos;
};

} // namespace mbgl
