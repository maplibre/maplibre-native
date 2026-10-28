#include <mln/text/glyph_cache.hpp>
#include <mln/util/hash.hpp>
#include <mln/util/std.hpp>
#include <mln/util/weak_immutable.hpp>

#include <algorithm>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace mln {

namespace {

constexpr std::size_t minSweep = 4096;

template <class Table>
void sweep(Table& table, std::size_t& nextSweep) {
    util::erase_if(table, [](const auto& entry) { return entry.second.expired(); });
    nextSweep = std::max(minSweep, 2 * table.size());
}

std::size_t hashOf(const Glyph& glyph) {
    const auto& m = glyph.metrics;
    const std::string_view bitmap(reinterpret_cast<const char*>(glyph.bitmap.data.get()), glyph.bitmap.bytes());
    return util::hash(glyph.id.hash, m.width, m.height, m.left, m.top, m.advance, m.isDoubleResolution, bitmap);
}

} // namespace

class GlyphCache::Impl {
public:
    std::mutex mutex;
    std::unordered_multimap<std::size_t, WeakImmutable<Glyph>> glyphs;
    std::size_t nextGlyphSweep = minSweep;
    std::unordered_multimap<std::size_t, std::weak_ptr<const Range>> ranges;
    std::size_t nextRangeSweep = minSweep;
};

GlyphCache::GlyphCache() = default;

GlyphCache::~GlyphCache() = default;

Immutable<Glyph> GlyphCache::intern(Glyph&& glyph) {
    const std::size_t hash = hashOf(glyph);
    std::scoped_lock lock(impl->mutex);
    auto [first, last] = impl->glyphs.equal_range(hash);
    for (auto it = first; it != last; ++it) {
        if (auto alive = it->second.lock(); alive && **alive == glyph) {
            return *alive;
        }
    }
    Immutable<Glyph> added = makeMutable<Glyph>(std::move(glyph));
    impl->glyphs.emplace(hash, added);
    if (impl->glyphs.size() >= impl->nextGlyphSweep) {
        sweep(impl->glyphs, impl->nextGlyphSweep);
    }
    return added;
}

std::shared_ptr<const GlyphCache::Range> GlyphCache::intern(Range&& range) {
    std::size_t hash = 0;
    for (const auto& [id, glyph] : range) {
        util::hash_combine(hash, glyph.get());
    }
    std::scoped_lock lock(impl->mutex);
    auto [first, last] = impl->ranges.equal_range(hash);
    for (auto it = first; it != last; ++it) {
        if (auto alive = it->second.lock(); alive && *alive == range) {
            return alive;
        }
    }
    auto added = std::make_shared<const Range>(std::move(range));
    impl->ranges.emplace(hash, added);
    if (impl->ranges.size() >= impl->nextRangeSweep) {
        sweep(impl->ranges, impl->nextRangeSweep);
    }
    return added;
}

std::size_t GlyphCache::size() const {
    std::scoped_lock lock(impl->mutex);
    return static_cast<std::size_t>(std::count_if(
        impl->glyphs.begin(), impl->glyphs.end(), [](const auto& entry) { return !entry.second.expired(); }));
}

} // namespace mln
