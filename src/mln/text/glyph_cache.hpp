#pragma once

#include <mln/text/glyph.hpp>
#include <mln/util/immutable.hpp>
#include <mln/util/weak_immutable.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace mln {

/// Parsed glyphs shared between the `GlyphManager`s of a process.
///
/// A glyph range is fetched per font stack, and every `Map` has its own `GlyphManager`, so the
/// same glyph is parsed and kept once per font stack and per `Map`. `intern()` returns the copy
/// that is already alive when there is one with the same id, metrics and bitmap. The cache only
/// holds weak references: a glyph is freed when the last `GlyphManager` using it drops it.
class GlyphCache {
public:
    GlyphCache();
    ~GlyphCache();
    GlyphCache(const GlyphCache&) = delete;
    GlyphCache& operator=(const GlyphCache&) = delete;

    /// The cache of the process. It lives as long as something holds it.
    static std::shared_ptr<GlyphCache> Get();

    using Range = std::map<GlyphID, Immutable<Glyph>>;

    /// Safe to call from any thread.
    Immutable<Glyph> intern(Glyph&&);

    /// The glyphs of a range, shared with the other owners of a range with the same glyphs. Intern the
    /// glyphs first: ranges are compared by the glyphs they point to. Safe to call from any thread.
    std::shared_ptr<const Range> intern(Range&&);

    /// The number of distinct glyphs alive.
    std::size_t size();

private:
    void sweep();

    std::mutex mutex;
    std::unordered_multimap<std::size_t, WeakImmutable<Glyph>> glyphs;
    std::unordered_multimap<std::size_t, std::weak_ptr<const Range>> ranges;
    std::size_t nextSweep;
    std::size_t nextRangeSweep;
};

} // namespace mln
