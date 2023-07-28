#pragma once

#include "glyph.hpp"

typedef struct FT_FaceRec_ *FT_Face;
typedef struct FT_LibraryRec_ *FT_Library;

namespace mbgl {

class FreeTypeLibrary {
public:
    friend class FreeTypeFace;
    FreeTypeLibrary();
    ~FreeTypeLibrary();

private:
    FT_Library library = nullptr;
};

class FreeTypeFace {
public:
    friend class HBShaper;
    explicit FreeTypeFace(const std::string &fontFileName, const FreeTypeLibrary &lib);
    explicit FreeTypeFace(const char *fontData, size_t fontDataSize, const FreeTypeLibrary &lib);
    ~FreeTypeFace();

    Glyph rasterizeGlyph(GlyphID glyphID);

    bool Valid() const { return valid; }

private:
    FT_Face face;
    int force_ucs2_charmap(FT_Face ftf);
    std::vector<uint8_t> memoryFile;

    bool valid = false;
};

} // namespace mbgl
