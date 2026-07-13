#include <mbgl/text/glyph.hpp>
#include <mbgl/util/i18n.hpp>

#include <algorithm>

namespace mbgl {

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

GlyphIDType genNewGlyphIDType() {
    static short glyphType = GlyphIDType::FontPBF;
    ++glyphType;
    if (glyphType == GlyphIDType::FontPBF) ++glyphType;
    // Intentionally casting to extend the enum's value space for runtime-generated IDs.
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return static_cast<GlyphIDType>(glyphType);
}

GlyphIDType genNewGlyphIDType(const std::string &url,
                              const FontStack &fontStack,
                              const std::vector<std::pair<uint32_t, uint32_t>> &pairs) {
    static std::map<std::string, std::map<std::string, std::map<std::size_t, GlyphIDType>>> glyphTypes;

    std::size_t hash = 0;
    for (auto &pair : pairs) {
        mbgl::util::hash_combine(hash, pair.first);
        mbgl::util::hash_combine(hash, pair.second);
    }

    auto family = fontStackToString(fontStack);

    auto type = glyphTypes[url][family][hash];
    if (type == GlyphIDType::FontPBF) {
        type = genNewGlyphIDType();
        glyphTypes[url][family][hash] = type;
    }

    return type;
}

// Note: this only works for the BMP
GlyphRange getGlyphRange(GlyphID glyph) {
    unsigned start = (glyph.complex.code / 256) * 256;
    unsigned end = (start + 255);
    start = std::min<unsigned int>(start, 65280);
    end = std::min<unsigned int>(end, 65535);
    return {start, end, glyph.complex.type};
}

} // namespace mbgl
