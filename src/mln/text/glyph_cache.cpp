#include <mln/text/glyph_cache.hpp>
#include <mln/util/hash.hpp>
#include <mln/util/std.hpp>

#include <algorithm>
#include <string_view>

namespace mln {

namespace {

constexpr std::size_t minSweep = 4096;

std::size_t hashOf(const Glyph& glyph) {
    const auto& m = glyph.metrics;
    const std::string_view bitmap(reinterpret_cast<const char*>(glyph.bitmap.data.get()), glyph.bitmap.bytes());
    return util::hash(glyph.id.hash, m.width, m.height, m.left, m.top, m.advance, m.isDoubleResolution, bitmap);
}

bool isSame(const Glyph& a, const Glyph& b) {
    return a.id.hash == b.id.hash && a.metrics == b.metrics && a.bitmap == b.bitmap;
}

bool isSame(const GlyphCache::Range& a, const GlyphCache::Range& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const auto& x, const auto& y) {
        return x.first.hash == y.first.hash && x.second == y.second;
    });
}

} // namespace

GlyphCache::GlyphCache()
    : nextSweep(minSweep),
      nextRangeSweep(minSweep) {}

GlyphCache::~GlyphCache() = default;

std::shared_ptr<GlyphCache> GlyphCache::Get() {
    static std::weak_ptr<GlyphCache> weak;
    static std::mutex mtx;

    std::scoped_lock lock(mtx);
    std::shared_ptr<GlyphCache> cache = weak.lock();
    if (!cache) {
        weak = cache = std::make_shared<GlyphCache>();
    }
    return cache;
}

Immutable<Glyph> GlyphCache::intern(Glyph&& glyph) {
    const std::size_t hash = hashOf(glyph);
    std::scoped_lock lock(mutex);
    auto [first, last] = glyphs.equal_range(hash);
    for (auto it = first; it != last; ++it) {
        if (auto alive = it->second.lock(); alive && isSame(**alive, glyph)) {
            return *alive;
        }
    }
    Immutable<Glyph> added = makeMutable<Glyph>(std::move(glyph));
    glyphs.emplace(hash, added);
    if (glyphs.size() >= nextSweep) {
        sweep();
    }
    return added;
}

std::shared_ptr<const GlyphCache::Range> GlyphCache::intern(Range&& range) {
    std::size_t hash = 0;
    for (const auto& [id, glyph] : range) {
        util::hash_combine(hash, glyph.get());
    }
    std::scoped_lock lock(mutex);
    auto [first, last] = ranges.equal_range(hash);
    for (auto it = first; it != last; ++it) {
        if (auto alive = it->second.lock(); alive && isSame(*alive, range)) {
            return alive;
        }
    }
    auto added = std::make_shared<const Range>(std::move(range));
    ranges.emplace(hash, added);
    if (ranges.size() >= nextRangeSweep) {
        util::erase_if(ranges, [](const auto& entry) { return entry.second.expired(); });
        nextRangeSweep = std::max(minSweep, 2 * ranges.size());
    }
    return added;
}

std::size_t GlyphCache::size() {
    std::scoped_lock lock(mutex);
    sweep();
    return glyphs.size();
}

void GlyphCache::sweep() {
    util::erase_if(glyphs, [](const auto& entry) { return entry.second.expired(); });
    nextSweep = std::max(minSweep, 2 * glyphs.size());
}

} // namespace mln
