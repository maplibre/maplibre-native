#pragma once

#include <mln/text/glyph.hpp>
#include <mln/text/glyph_range.hpp>

#include <string>
#include <vector>

namespace mln {

std::vector<Glyph> parseGlyphPBF(const GlyphRange&, const std::string& data);

} // namespace mln
