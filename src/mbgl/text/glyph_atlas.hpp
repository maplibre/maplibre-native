#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/gfx/dynamic_texture.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

struct GlyphPosition {
    gfx::TextureHandle handle;
    GlyphMetrics metrics;
};

using GlyphPositionMap = std::map<GlyphID, GlyphPosition>;
using GlyphPositions = std::map<FontStackHash, GlyphPositionMap>;

GlyphPositions uploadGlyphs(const GlyphMap&);

} // namespace mbgl
