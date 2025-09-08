#pragma once

#include <utility>
#include <cstdint>
#include <unordered_set>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/font_stack.hpp>

namespace mbgl {

enum GlyphIDType : uint16_t {
    FontPBF = 0x00,
};

GlyphIDType genNewGlyphIDType(const std::string &url,
                              const FontStack &fontStack,
                              const std::vector<std::pair<uint32_t, uint32_t>> &pairs);

class GlyphRange {
public:
    uint16_t first = 0;
    uint16_t second = 0;

    GlyphIDType type = GlyphIDType::FontPBF;

    GlyphRange(uint32_t first_, uint32_t second_, GlyphIDType type_ = FontPBF);

    bool operator==(const GlyphRange &other) const;
    bool operator<(const GlyphRange &other) const;
};

constexpr uint32_t GLYPHS_PER_GLYPH_RANGE = 256;
constexpr uint32_t GLYPH_RANGES_PER_FONT_STACK = 256;
// 256 - 126 ranges skipped w/ i18n::allowsFixedWidthGlyphGeneration
constexpr uint32_t NON_IDEOGRAPH_GLYPH_RANGES_PER_FONT_STACK = 130;

} // end namespace mbgl

namespace std {

template <>
struct hash<mbgl::GlyphRange> {
    std::size_t operator()(const mbgl::GlyphRange &range) const { return mbgl::util::hash(range.first, range.second); }
};

} // namespace std
