#pragma once

#include <mln/style/types.hpp>
#include <mln/text/glyph_range.hpp>
#include <mln/util/font_stack.hpp>

#include <exception>

namespace mln {

class GlyphManagerObserver {
public:
    virtual ~GlyphManagerObserver() = default;

    virtual void onGlyphsLoaded(const FontStack&, const GlyphRange&) {}
    virtual void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) {}
    virtual void onGlyphsRequested(const FontStack&, const GlyphRange&) {}
};

} // namespace mln
