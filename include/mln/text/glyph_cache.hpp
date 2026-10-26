#pragma once

#include <mln/text/glyph.hpp>
#include <mln/util/immutable.hpp>

#include <cstddef>
#include <map>
#include <memory>

namespace mln {

// Parsed glyphs shared by the renderers given the same GlyphCache, matched by contents. It holds weak
// references only: a glyph is freed with its last owner. It can be shared between threads.
class GlyphCache {
public:
    GlyphCache();
    ~GlyphCache();
    GlyphCache(const GlyphCache&) = delete;
    GlyphCache& operator=(const GlyphCache&) = delete;

    using Range = std::map<GlyphID, Immutable<Glyph>>;

    Immutable<Glyph> intern(Glyph&&);

    // Ranges are matched by the glyphs they point to: intern the glyphs first.
    std::shared_ptr<const Range> intern(Range&&);

    std::size_t size();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mln
