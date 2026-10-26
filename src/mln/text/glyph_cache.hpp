#pragma once

#include <mln/text/glyph.hpp>
#include <mln/util/immutable.hpp>
#include <mln/util/weak_immutable.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace mln {

// Parsed glyphs shared by the GlyphManagers of a process, matched by contents. The cache holds weak
// references only: a glyph is freed with its last owner.
class GlyphCache {
public:
    GlyphCache();
    ~GlyphCache();
    GlyphCache(const GlyphCache&) = delete;
    GlyphCache& operator=(const GlyphCache&) = delete;

    static std::shared_ptr<GlyphCache> Get();

    Immutable<Glyph> intern(Glyph&&);

    std::size_t size();

private:
    void sweep();

    std::mutex mutex;
    std::unordered_multimap<std::size_t, WeakImmutable<Glyph>> glyphs;
    std::size_t nextSweep;
};

} // namespace mln
