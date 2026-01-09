#pragma once

#include <mbgl/text/glyph_range.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/util/font_stack.hpp>
#include <mbgl/util/rect.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/util.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <set>

namespace mbgl {

union GlyphID {
    char32_t hash;
    struct {
        char16_t code;
        GlyphIDType type;
    } complex;

    GlyphID(int codepoint) {
        complex.type = FontPBF;
        complex.code = codepoint;
    }
    GlyphID(uint32_t codepoint) {
        complex.type = FontPBF;
        complex.code = codepoint;
    }
    GlyphID(char16_t codepoint) {
        complex.type = FontPBF;
        complex.code = codepoint;
    }

    GlyphID(char16_t index, GlyphIDType t) {
        complex.type = t;
        complex.code = index;
    }

    operator char16_t() { return complex.code; }
    operator char32_t() { return hash; }
    bool operator<(const GlyphID &other) const { return hash < other.hash; }
    bool operator>(const GlyphID &other) const { return hash > other.hash; }

    bool operator<(const uint16_t &other) const { return hash < other; }
    bool operator>(const uint16_t &other) const { return hash > other; }
};

using GlyphIDs = std::set<GlyphID>;

// Note: this only works for the BMP
GlyphRange getGlyphRange(GlyphID glyph);

struct GlyphMetrics {
    uint32_t width = 0;
    uint32_t height = 0;
    int32_t left = 0;
    int32_t top = 0;
    uint32_t advance = 0;
};

inline bool operator==(const GlyphMetrics &lhs, const GlyphMetrics &rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height && lhs.left == rhs.left && lhs.top == rhs.top &&
           lhs.advance == rhs.advance;
}

class Glyph {
public:
    // We're using this value throughout the Mapbox GL ecosystem. If this is
    // different, the glyphs also need to be reencoded.
    static constexpr const uint8_t borderSize = 3;

    GlyphID id = 0;

    // A signed distance field of the glyph with a border (see above).
    AlphaImage bitmap;

    // Glyph metrics
    GlyphMetrics metrics;
};

using Glyphs = std::map<GlyphID, std::optional<Immutable<Glyph>>>;
using GlyphMap = std::map<FontStackHash, Glyphs>;

class PositionedGlyph {
public:
    explicit PositionedGlyph(GlyphID glyph_,
                             float x_,
                             float y_,
                             bool vertical_,
                             FontStackHash font_,
                             float scale_,
                             Rect<uint16_t> rect_,
                             GlyphMetrics metrics_,
                             std::optional<std::string> imageID_,
                             std::size_t sectionIndex_ = 0)
        : glyph(glyph_),
          x(x_),
          y(y_),
          vertical(vertical_),
          font(font_),
          scale(scale_),
          rect(rect_),
          metrics(metrics_),
          imageID(std::move(imageID_)),
          sectionIndex(sectionIndex_) {}

    GlyphID glyph = 0;
    float x = 0;
    float y = 0;
    bool vertical = false;
    FontStackHash font = 0;
    float scale = 0.0;
    Rect<uint16_t> rect;
    GlyphMetrics metrics;
    std::optional<std::string> imageID;
    // Maps positioned glyph to TaggedString section
    std::size_t sectionIndex;
};

enum class WritingModeType : uint8_t;

struct PositionedLine {
    std::vector<PositionedGlyph> positionedGlyphs;
    float lineOffset = 0.0;
};

class Shaping {
public:
    Shaping() = default;
    explicit Shaping(float x, float y, WritingModeType writingMode_)
        : top(y),
          bottom(y),
          left(x),
          right(x),
          writingMode(writingMode_) {}
    std::vector<PositionedLine> positionedLines;
    float top = 0;
    float bottom = 0;
    float left = 0;
    float right = 0;
    WritingModeType writingMode;
    explicit operator bool() const {
        return std::ranges::any_of(positionedLines, [](const auto &line) { return !line.positionedGlyphs.empty(); });
    }
    // The y offset *should* be part of the font metadata.
    static constexpr int32_t yOffset = -17;
    bool verticalizable = false;
    bool iconsInText = false;
};

enum class WritingModeType : uint8_t {
    None = 0,
    Horizontal = 1 << 0,
    Vertical = 1 << 1,
};

// style defined faces
struct FontFace {
    using Range = std::pair<uint32_t, uint32_t>;
    GlyphIDType type;          // an unique glyph id
    std::string name;          // font face name
    std::string url;           // font file url
    std::vector<Range> ranges; // unicode ranges

    FontFace() = default;
    FontFace(const std::string &name_, const std::string &url_, const std::vector<Range> &ranges_)
        : type(FontPBF),
          name(name_),
          url(url_),
          ranges(ranges_) {}

    FontFace(const std::string &name_, const std::string &url_, std::vector<Range> &&ranges_)
        : type(FontPBF),
          name(name_),
          url(url_),
          ranges(std::move(ranges_)) {}

    auto valid() const -> bool { return !name.empty() && !url.empty() && !ranges.empty(); }
};

using FontFaces = std::vector<FontFace>;

struct HBShapeRequest {
    std::u16string str;
    FontStack fontStack;
    GlyphIDType type;

    HBShapeRequest(const std::u16string &str_, const FontStack &fontStack_, GlyphIDType type_)
        : str(str_),
          fontStack(fontStack_),
          type(type_) {}
};

using HBShapeRequests = std::map<FontStack, std::map<GlyphIDType, std::set<std::u16string>>>;

struct GlyphDependencies {
    std::map<FontStack, GlyphIDs> glyphs;
    HBShapeRequests shapes;
};

using GlyphRangeDependencies = std::map<FontStack, std::unordered_set<GlyphRange>>;

struct GlyphPosition {
    Rect<uint16_t> rect;
    GlyphMetrics metrics;
};

using GlyphPositionMap = std::map<GlyphID, GlyphPosition>;
using GlyphPositions = std::map<FontStackHash, GlyphPositionMap>;

} // end namespace mbgl
