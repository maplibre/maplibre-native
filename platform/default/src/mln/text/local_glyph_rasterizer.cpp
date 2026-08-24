#include <mln/text/local_glyph_rasterizer.hpp>

namespace mln {

class LocalGlyphRasterizer::Impl {};

LocalGlyphRasterizer::LocalGlyphRasterizer(const std::optional<std::string>&) {}

LocalGlyphRasterizer::~LocalGlyphRasterizer() = default;

bool LocalGlyphRasterizer::canRasterizeGlyph(const FontStack&, GlyphID) {
    return false;
}

Glyph LocalGlyphRasterizer::rasterizeGlyph(const FontStack&, GlyphID) {
    return Glyph();
}

} // namespace mln
