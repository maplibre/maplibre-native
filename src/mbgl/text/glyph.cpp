#include <mbgl/text/glyph.hpp>
#include <mbgl/util/i18n.hpp>

namespace mbgl {

#ifdef MLN_TEXT_SHAPING_HARFBUZZ

GlyphRange::GlyphRange(uint32_t first_, uint32_t second_, GlyphIDType type_)
    : first((uint16_t)first_),
      second((uint16_t)second_),
      type(type_) {}

bool GlyphRange::operator==(const GlyphRange &other) const {
    return first == other.first && second == other.second && type == other.type;
}

bool GlyphRange::operator<(const GlyphRange &other) const {
    if (first < other.first) return true;
    if (first > other.first) return false;

    if (second < other.second) return true;
    if (second > other.second) return false;

    return type < other.type;
}

const std::string getGlyphRangeName(GlyphIDType type) {
    switch (type) {
        case GlyphIDType::Khmer:
            return "khmer";

        case GlyphIDType::Myanmar:
            return "myanmar";

        case GlyphIDType::Devanagari:
            return "devanagari";

        default:
            return "";
    }
}

#endif

// Note: this only works for the BMP
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
GlyphRange getGlyphRange(GlyphID glyph) {
    unsigned start = (glyph.complex.code / 256) * 256;
    unsigned end = (start + 255);
    if (start > 65280) start = 65280;
    if (end > 65535) end = 65535;
    return {start, end, glyph.complex.type};
}
#else
GlyphRange getGlyphRange(GlyphID glyph) {
    unsigned start = (glyph / 256) * 256;
    unsigned end = (start + 255);
    if (start > 65280) start = 65280;
    if (end > 65535) end = 65535;
    return {start, end};
}
#endif

} // namespace mbgl
