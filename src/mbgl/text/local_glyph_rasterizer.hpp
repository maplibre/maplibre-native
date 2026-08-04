#pragma once

#include <mbgl/text/glyph.hpp>

namespace mbgl {

/*
    Given a font stack and a glyph ID, platform-specific implementations of
    LocalGlyphRasterizer will decide which, if any, local fonts to use and
    then generate a matching glyph object with a greyscale rasterization of
    the glyph and appropriate metrics. GlyphManager will then use TinySDF to
    transform the rasterized bitmap into an SDF.

    The shared default codepoint filter is
    util::i18n::allowsFixedWidthGlyphGeneration.
    Following maplibre-gl-js, platform implementations use 2x bitmaps with
    1x metrics via GlyphMetrics::isDoubleResolution.

    It is left to platform-specific implementation to decide how best to
    map a FontStack to a particular rasterization (font, weight, style).

    The default implementation simply refuses to rasterize any glyphs.
*/

class LocalGlyphRasterizer {
public:
    virtual ~LocalGlyphRasterizer();
    LocalGlyphRasterizer(const std::optional<std::string>& fontFamily = std::nullopt);

    // virtual so that test harness can override platform-specific behavior
    virtual bool canRasterizeGlyph(const FontStack&, GlyphID);
    virtual Glyph rasterizeGlyph(const FontStack&, GlyphID);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
