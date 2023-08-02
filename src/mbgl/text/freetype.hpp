#pragma once

#include "glyph.hpp"

struct FT_FaceRec_;
struct FT_LibraryRec_;

using FT_Face = FT_FaceRec_*;
using FT_Library = FT_LibraryRec_*;

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
