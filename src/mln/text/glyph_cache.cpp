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

class GlyphCache::Impl {
public:
    void sweep() {
        util::erase_if(glyphs, [](const auto& entry) { return entry.second.expired(); });
        nextSweep = std::max(minSweep, 2 * glyphs.size());
    }

    std::mutex mutex;
    std::unordered_multimap<std::size_t, WeakImmutable<Glyph>> glyphs;
    std::size_t nextSweep = minSweep;
    std::unordered_multimap<std::size_t, std::weak_ptr<const Range>> ranges;
    std::size_t nextRangeSweep = minSweep;
};

GlyphCache::GlyphCache()
    : impl(std::make_unique<Impl>()) {}

GlyphCache::~GlyphCache() = default;

Immutable<Glyph> GlyphCache::intern(Glyph&& glyph) {
    const std::size_t hash = hashOf(glyph);
    std::scoped_lock lock(impl->mutex);
    auto [first, last] = impl->glyphs.equal_range(hash);
    for (auto it = first; it != last; ++it) {
        if (auto alive = it->second.lock(); alive && isSame(**alive, glyph)) {
            return *alive;
        }
    }
    Immutable<Glyph> added = makeMutable<Glyph>(std::move(glyph));
    impl->glyphs.emplace(hash, added);
    if (impl->glyphs.size() >= impl->nextSweep) {
        impl->sweep();
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
        if (auto alive = it->second.lock(); alive && isSame(*alive, range)) {
            return alive;
        }
    }
    auto added = std::make_shared<const Range>(std::move(range));
    impl->ranges.emplace(hash, added);
    if (impl->ranges.size() >= impl->nextRangeSweep) {
        util::erase_if(impl->ranges, [](const auto& entry) { return entry.second.expired(); });
        impl->nextRangeSweep = std::max(minSweep, 2 * impl->ranges.size());
    }
    return added;
}

std::size_t GlyphCache::size() {
    std::scoped_lock lock(impl->mutex);
    impl->sweep();
    return impl->glyphs.size();
}

} // namespace mln
