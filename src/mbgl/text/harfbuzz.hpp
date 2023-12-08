#pragma once

#include <harfbuzz_wrap.hpp>
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

    bool Valid() { return shaper->Valid(); }

private:
    GlyphIDType type;
    std::unique_ptr<HBShaperWrap> shaper = nullptr;
};

} // namespace mbgl
