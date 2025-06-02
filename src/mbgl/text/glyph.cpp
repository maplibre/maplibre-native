#include <algorithm>
#include <mbgl/text/glyph.hpp>

namespace mbgl {

// Note: this only works for the BMP
GlyphRange getGlyphRange(GlyphID glyph) {
    unsigned start = (glyph / 256) * 256;
    unsigned end = (start + 255);
    start = std::min<unsigned int>(start, 65280);
    end = std::min<unsigned int>(end, 65535);
    return {start, end};
}

} // namespace mbgl
