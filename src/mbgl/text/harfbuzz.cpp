#include <mbgl/text/harfbuzz.hpp>

#include <hb-ft.h>

namespace mbgl
{

struct InternalHBLangInfo {
    std::string fontFileName;
    std::string language;
    hb_script_t script;
    hb_direction_t direction;
};

std::map<GlyphIDType, InternalHBLangInfo> HBShaper::internalInfos = {
    { GlyphIDType::Khmer, { "khmer.ttf", "kh", HB_SCRIPT_KHMER, HB_DIRECTION_LTR } },
    { GlyphIDType::Myanmar, { "Myanmar.ttf", "my", HB_SCRIPT_MYANMAR, HB_DIRECTION_LTR } },
    { GlyphIDType::Devanagari, { "Devanagari.ttf", "hi", HB_SCRIPT_DEVANAGARI, HB_DIRECTION_LTR } },
};

HBShaper::HBShaper(GlyphIDType _type, const FreeTypeLibrary &lib) : face(internalInfos[_type].fontFileName , lib) , type(_type) {
    if (!face.Valid())
        return;
    hbInfo = &internalInfos[_type];
    
    font = hb_ft_font_create(face.face, NULL);
    buffer = hb_buffer_create();

    hb_buffer_allocation_successful(buffer);
}


HBShaper::HBShaper(GlyphIDType _type, const std::string &fontFileData, const FreeTypeLibrary &lib) : face(fontFileData.data(), fontFileData.size() , lib) , type(_type) {
    if (!face.Valid())
        return;
    
    hbInfo = &internalInfos[_type];
    
    font = hb_ft_font_create(face.face, NULL);
    buffer = hb_buffer_create();

    hb_buffer_allocation_successful(buffer);
}


HBShaper::~HBShaper() {
    if (!face.valid)
        return;
    hb_buffer_destroy(buffer);
    hb_font_destroy(font);
}

void HBShaper::CreateComplexGlyphIDs(const std::string &text, std::vector<GlyphID> &glyphIDs, std::vector<HBShapeAdjust> &adjusts) {
    // Setup harfbuzz
    hb_buffer_reset(buffer);

    hb_buffer_set_direction(buffer, hbInfo->direction);
    hb_buffer_set_script(buffer, hbInfo->script);
    hb_buffer_set_language(buffer, hb_language_from_string(hbInfo->language.c_str(), (int)hbInfo->language.size()));
    size_t length = text.size();

    hb_buffer_add_utf8(buffer, text.c_str(), (int)length, 0, (int)length);

    // harfbuzz shaping
    hb_shape(font, buffer, NULL, 0);

    // Get Harfbuzz adjustion
    uint32_t glyphCount;
    hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
    hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);
    
    glyphIDs.reserve(glyphCount);
    adjusts.reserve(glyphCount);
    
    for(uint32_t i = 0; i < glyphCount; ++i) {
        glyphIDs.emplace_back(glyphInfo[i].codepoint, type);
        
        uint32_t x_advance = glyphPos[i].x_advance / 64;
        uint32_t x_offset = glyphPos[i].x_offset / 64;
        uint32_t y_offset = glyphPos[i].y_offset / 64;
        
        adjusts.emplace_back(x_offset, y_offset, x_advance);
    }

}

void HBShaper::CreateComplexGlyphIDs(const std::u16string &text, std::vector<GlyphID> &glyphIDs, std::vector<HBShapeAdjust> &adjusts) {
    // Setup harfbuzz
    hb_buffer_reset(buffer);

    hb_buffer_set_direction(buffer, hbInfo->direction);
    hb_buffer_set_script(buffer, hbInfo->script);
    hb_buffer_set_language(buffer, hb_language_from_string(hbInfo->language.c_str(), (int)hbInfo->language.size()));
    size_t length = text.size();

    hb_buffer_add_utf16(buffer, (const uint16_t *)text.c_str(), (int)length, 0, (int)length);

    // harfbuzz shaping
    hb_shape(font, buffer, NULL, 0);

    // Get Harfbuzz adjustion
    uint32_t glyphCount;
    hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
    hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);
    
    glyphIDs.reserve(glyphCount);
    adjusts.reserve(glyphCount);
    
    for(uint32_t i = 0; i < glyphCount; ++i) {
        glyphIDs.emplace_back(glyphInfo[i].codepoint, type);
        
        float x_advance = glyphPos[i].x_advance / 64.0;
        float x_offset = glyphPos[i].x_offset / 64.0;
        float y_offset = -glyphPos[i].y_offset / 64.0f;
        
        adjusts.emplace_back(x_offset, y_offset, x_advance);
    }
}


} // namespace mbgl
