#pragma once

#include <mln/text/glyph.hpp>
#include <mln/util/immutable.hpp>

#include <cstddef>
#include <map>
#include <memory>

namespace mln {

// Keeps one copy of equal glyphs and glyph ranges. It only holds weak references, so it never keeps a glyph
// alive. Thread-safe.
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

    // Glyphs alive, not counting ranges. Walks the whole table.
    std::size_t size() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl = std::make_unique<Impl>();
};

} // namespace mln
