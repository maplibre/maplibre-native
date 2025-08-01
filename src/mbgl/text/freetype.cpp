#include "freetype.hpp"

#include <mbgl/text/glyph.hpp>

#include <cstring>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace mbgl {

//
// FreeTypeLibray
//

#define SDF_FONT_SIZE 24

FreeTypeLibrary::FreeTypeLibrary() {
    FT_Init_FreeType(&library);
}

FreeTypeLibrary::~FreeTypeLibrary() {
    FT_Done_FreeType(library);
}

// FreeTypeFace

FreeTypeFace::FreeTypeFace(const std::string &fontFileName, const FreeTypeLibrary &lib) {
    FT_Error error = FT_New_Face(lib.library, fontFileName.c_str(), 0, &face);
    valid = (error == 0);
    if (!valid) return;

    force_ucs2_charmap(face);
    FT_Set_Char_Size(face, 0, SDF_FONT_SIZE * 64, 72, 72);
}

FreeTypeFace::FreeTypeFace(const char *fontData, size_t fontDataSize, const FreeTypeLibrary &lib) {
    memoryFile.resize(fontDataSize);
    std::memcpy(memoryFile.data(), fontData, fontDataSize);
    FT_Error error = FT_New_Memory_Face(lib.library, memoryFile.data(), (FT_Long)fontDataSize, 0, &face);
    valid = (error == 0);
    if (!valid) return;

    force_ucs2_charmap(face);
    FT_Set_Char_Size(face, 0, SDF_FONT_SIZE * 64, 72, 72);
}

FreeTypeFace::~FreeTypeFace() {
    if (valid) FT_Done_Face(face);
}

int FreeTypeFace::force_ucs2_charmap(FT_Face ftf) {
    for (int i = 0; i < ftf->num_charmaps; i++) {
        if (((ftf->charmaps[i]->platform_id == 0) && (ftf->charmaps[i]->encoding_id == 3)) ||
            ((ftf->charmaps[i]->platform_id == 3) && (ftf->charmaps[i]->encoding_id == 1))) {
            return FT_Set_Charmap(ftf, ftf->charmaps[i]);
        }
    }
    return -1;
}

Glyph FreeTypeFace::rasterizeGlyph(const GlyphID &glyph) {
    Glyph fixedMetrics;

    FT_Int32 flags = FT_LOAD_DEFAULT;

    FT_Load_Glyph(face,
                  (FT_UInt)glyph.complex.code, // the glyph_index in the font file
                  flags);

    FT_GlyphSlot slot = face->glyph;
    FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

    const FT_Bitmap &ftBitmap = slot->bitmap;

    fixedMetrics.id = glyph;

    Size size(ftBitmap.width + Glyph::borderSize * 2, ftBitmap.rows + Glyph::borderSize * 2);

    fixedMetrics.metrics.width = size.width;
    fixedMetrics.metrics.height = size.height;
    fixedMetrics.metrics.left = slot->bitmap_left;
    fixedMetrics.metrics.top = slot->bitmap_top - SDF_FONT_SIZE - Glyph::borderSize;
    fixedMetrics.metrics.advance = (uint32_t)(slot->metrics.horiAdvance / 64);

    // Copy alpha values from RGBA bitmap into the AlphaImage output
    fixedMetrics.bitmap = AlphaImage(size);

    for (uint32_t h = 0; h < ftBitmap.rows; ++h) {
        std::memcpy(&fixedMetrics.bitmap.data[size.width * (h + Glyph::borderSize) + Glyph::borderSize],
                    &ftBitmap.buffer[ftBitmap.width * h],
                    ftBitmap.width);
    }

    return fixedMetrics;
}

} // namespace mbgl
