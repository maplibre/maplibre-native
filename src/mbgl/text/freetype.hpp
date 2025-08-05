#pragma once

#include <mbgl/text/glyph.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

struct FT_FaceRec_;
struct FT_LibraryRec_;

using FT_Face = FT_FaceRec_ *;
using FT_Library = FT_LibraryRec_ *;

namespace mbgl {

class FreeTypeLibrary {
public:
    friend class FreeTypeFace;
    FreeTypeLibrary();
    ~FreeTypeLibrary();

private:
    FT_Library library = nullptr;
};

// call back format: width, height, left, top, advance, bitmap data
using GlyphCallBack = std::function<void(uint32_t, uint32_t, int, int, uint32_t, unsigned char *)>;

class FreeTypeFace {
public:
    explicit FreeTypeFace(const std::string &fontFileName, const FreeTypeLibrary &lib);
    explicit FreeTypeFace(const char *fontData, size_t fontDataSize, const FreeTypeLibrary &lib);
    ~FreeTypeFace();

    Glyph rasterizeGlyph(const GlyphID &glyph);

    bool isValid() const { return valid; }

    FT_Face getFace() { return face; }

private:
    FT_Face face;
    int force_ucs2_charmap(FT_Face ftf);
    std::vector<uint8_t> memoryFile;

    bool valid = false;
};

} // namespace mbgl
